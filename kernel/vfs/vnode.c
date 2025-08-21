#include <vfs/vnode.h>
#include <lib/string.h>
#include <memory/memory.h>
#include <vfs/vfs.h>
#include <fs/fat32.h>
#include <drivers/emmc.h>
#include <lib/printf.h>

VfsRoot *get_root(const char *path)
{
    for (unsigned long i = 0; i < get_vfs()->roots_count; i++)
    {
        if (path[0] == get_vfs()->roots[i].id)
            return &get_vfs()->roots[i];
    }

    return 0;
}

bool check_open_vnode(VfsRoot *root, VNode *node)
{
    for (unsigned long i = 0; i < root->nodes_count; i++)
    {
        if (root->open_nodes[i] && strcmp(root->open_nodes[i]->path, node->path) == 0)
            return true;
    }

    return false;
}

void open_vnode(VfsRoot *root, VNode *node)
{
    VNode **found = 0;
    for (unsigned long i = 0; i < root->nodes_count; i++)
    {
        if (root->open_nodes[i] == 0)
        {
            found = &root->open_nodes[i];
            break;
        }
    }

    if (!found && root->nodes_capacity == root->nodes_count)
    {
        root->nodes_capacity *= 2;

        VNode **new_nodes = malloc(root->nodes_capacity * sizeof(VNode));
        memcpy(new_nodes, root->open_nodes, root->nodes_count);

        free(root->open_nodes);
        root->open_nodes = new_nodes;
    }

    if (found)
        *found = node;
    else
    {
        root->open_nodes[root->nodes_count] = node;
        root->nodes_count++;
    }
}

bool get_fat_node_from_path(VfsRoot *root, FatFSNode *node, bool found_node, const char *path, unsigned long entry_path_size, unsigned long start)
{
    FatFSNode *nodes = 0;
    FatFS *fatfs = (FatFS *)root->fs;
    unsigned long nodes_count = GetFatEntries(root->fs, !found_node ? fatfs->root_cluster : node->content_cluster, &nodes);

    for (unsigned long i = 0; i < nodes_count; i++)
    {
        if (strlen(nodes[i].name) != entry_path_size)
            continue;
        if (memcmp(nodes[i].name, &path[start], entry_path_size) != 0)
            continue;

        memcpy(node, &nodes[i], sizeof(FatFSNode));
        free(nodes);
        return true;
    }

    if (nodes != 0)
        free(nodes);
    return false;
}

bool is_directory(void *node, enum Filesystems fs)
{
    switch (fs)
    {
    case FAT32:
        return ((FatFSNode *)node)->entry.attrs & 0x10;
    default:
        return false;
    }
}

void *entry_from_path(const char *path, VfsRoot *root)
{
    bool found_node = false;
    bool finishing = false;
    void *node = malloc(sizeof(FatFSNode));
    unsigned long path_size = strlen(path);

    if (path_size == 0)
    {
        LOG("Path not specified.\n");
        return 0;
    }

    if (path_size == 1)
    {
        LOG("Cannot open root.\n");
        return 0;
    }

    if (path[path_size - 1] == '/')
    {
        LOG("Cannot open directories.\n");
        return 0;
    }

    for (unsigned long start = 1; start < path_size;)
    {
        unsigned long end = start;
        while (path[end] != '/')
        {
            if (path[end] == '\0')
            {
                finishing = true;
                break;
            }
            end++;
        }

        unsigned long entry_path_size = end - start;

        if (root->fs_type == FAT32)
            found_node = get_fat_node_from_path(root, node, found_node, path, entry_path_size, start);

        if (!found_node)
        {
            LOG("Cannot find: \"");
            for (unsigned long i = 0; i < entry_path_size; i++)
                LOG("%c", path[start + i]);
            LOG("\"\n");
            free(node);
            return 0;
        }

        if (!finishing && !is_directory(node, root->fs_type))
        {
            LOG("File used as a directory.\n");
            return 0;
        }

        if (finishing && is_directory(node, root->fs_type))
        {
            LOG("Cannot open directories.\n");
            return 0;
        }

        start = end + 1;
    }

    // safety measure in case of anything
    if (!found_node)
        return 0;
    return node;
}

VNode *OpenFile(const char *path)
{
    Printf("0");
    unsigned long len = strlen(path);
    if (len == 0)
        return 0;

    VfsRoot *root = get_root(path);
    if (!root)
        return 0;

    Printf("1");
    VNode *node = malloc(sizeof(VNode));
    node->path = path;
    node->root = root;
    if (check_open_vnode(root, node))
    {
        LOG("File is already open.\n");
        return 0;
    }

    Printf("2");

    void *entry = entry_from_path(path, root);
    node->data = entry;
    if (is_directory(node, node->root->fs_type))
        node->type = VNODE_DIRECTORY;
    else
        node->type = VNODE_FILE;

    Printf("3");
    open_vnode(root, node);
    Printf("4");
    return node;
}

void CloseFile(VNode *node)
{
    VfsRoot *root = get_root(node->path);
    if (!root)
        return;

    for (unsigned long i = 0; i < root->nodes_count; i++)
    {
        if (strcmp(root->open_nodes[i]->path, node->path) == 0)
        {
            root->open_nodes[i] = 0;
            return;
        }
    }
}

void SeekInFile(VNode *node, unsigned long seek, enum FSeek_Types type)
{
    switch (type)
    {
    case SEEK_SET:
        node->seek = seek;
        break;

    case SEEK_CUR:
        node->seek += seek;
        break;

    default:
        break;
    }
}

unsigned long ReadFile(void *write_buf, unsigned long size, VNode *node)
{
    if (is_directory(node, node->root->fs_type))
        return 0;

    void *buf = write_buf;
    if (node->root->fs_type == FAT32)
    {
        FatFSNode *entry = (FatFSNode *)node->data;
        FatFS *fs = (FatFS *)node->root->fs;
        if (size > entry->entry.size)
            size = entry->entry.size;

        void *init_buf = buf;
        unsigned long cl_size = FatClusterSize(fs);
        unsigned long first_cluster = node->seek / cl_size;
        unsigned long last_cluster = (node->seek + size - 1) / cl_size;
        unsigned long start = node->seek % cl_size;
        unsigned long end = (node->seek + size) % cl_size;

        unsigned long bytes_left = size;

        for (unsigned long cluster = first_cluster; cluster <= last_cluster; cluster++)
        {
            unsigned char *read = ReadFatNodeAt(*entry, cluster);
            unsigned char *cpy_start = read;
            unsigned long cpy_len = cl_size;

            if (cluster == first_cluster)
            {
                cpy_start += start;
                cpy_len -= start;
            }

            if (cluster == last_cluster && end != 0)
                // edge case
                cpy_len = end;

            if (cpy_len > bytes_left)
                cpy_len = bytes_left;

            memcpy(buf, cpy_start, cpy_len);
            free(read);

            buf += cpy_len;
            bytes_left -= cpy_len;
        }

        return buf - init_buf;
    }

    return 0;
}

void WriteToFile(void *buf, unsigned long size, VNode *node)
{
    if (is_directory(node, node->root->fs_type))
        return;

    if (node->root->fs_type == FAT32)
    {
        FatFSNode *entry = (FatFSNode *)node->data;
        WriteToFatNode(entry, buf, size);
    }
}
