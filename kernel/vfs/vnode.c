#include <vfs/vnode.h>
#include <lib/string.h>
#include <memory/memory.h>
#include <vfs/vfs.h>
#include <fs/fat32.h>
#include <drivers/emmc.h>
#include <lib/printf.h>

vfs_root_t *get_root(const char *path)
{
    for (unsigned long i = 0; i < get_vfs()->roots_count; i++)
    {
        if (path[0] == get_vfs()->roots[i].id)
            return &get_vfs()->roots[i];
    }

    return 0;
}

bool check_open_vnode(vfs_root_t *root, vnode_t *node)
{
    for (unsigned long i = 0; i < root->nodes_count; i++)
    {
        if (strcmp(root->open_nodes[i]->path, node->path) == 0)
            return true;
    }

    return false;
}

void open_vnode(vfs_root_t *root, vnode_t *node)
{
    vnode_t **found = 0;
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

        vnode_t **new_nodes = malloc(root->nodes_capacity * sizeof(vnode_t));
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

bool get_fat_node_from_path(vfs_root_t *root, fatfs_node_t *node, bool found_node, const char *path, unsigned long entry_path_size, unsigned long start)
{
    fatfs_node_t *nodes = 0;
    fatfs_t *fatfs = (fatfs_t *)root->fs;
    unsigned long nodes_count = get_fatentries(root->fs, !found_node ? fatfs->root_cluster : node->content_cluster, &nodes);

    for (unsigned long i = 0; i < nodes_count; i++)
    {
        if (strlen(nodes[i].name) != entry_path_size)
            continue;
        if (memcmp(nodes[i].name, &path[start], entry_path_size) != 0)
            continue;

        memcpy(node, &nodes[i], sizeof(fatfs_node_t));
        free(nodes);
        return true;
    }

    if (nodes != 0)
        free(nodes);
    return false;
}

bool is_directory(void *node, enum FileSystems fs)
{
    switch (fs)
    {
    case FAT32:
        return ((fatfs_node_t *)node)->entry.attrs & 0x10;
    default:
        return false;
    }
}

void *entry_from_path(const char *path, vfs_root_t *root)
{
    bool found_node = false;
    bool finishing = false;
    void *node = malloc(sizeof(fatfs_node_t));
    unsigned long path_size = strlen(path);

    if (path_size == 0)
    {
        printf("Path not specified.\n");
        return 0;
    }

    if (path_size == 1)
    {
        printf("Cannot open root.\n");
        return 0;
    }

    if (path[path_size - 1] == '/')
    {
        printf("Cannot open directories.\n");
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
            printf("Cannot find: \"");
            for (unsigned long i = 0; i < entry_path_size; i++)
                printf("%c", path[start + i]);
            printf("\"\n");
            free(node);
            return 0;
        }

        if (!finishing && !is_directory(node, root->fs_type))
        {
            printf("File used as a directory.\n");
            return 0;
        }

        if (finishing && is_directory(node, root->fs_type))
        {
            printf("Cannot open directories.\n");
            return 0;
        }

        start = end + 1;
    }

    // safety measure in case of anything
    if (!found_node)
        return 0;
    return node;
}

vnode_t *fopen(const char *path)
{
    unsigned long len = strlen(path);
    if (len == 0)
        return 0;

    vfs_root_t *root = get_root(path);
    if (!root)
        return 0;

    vnode_t *node = malloc(sizeof(vnode_t));
    node->path = path;
    node->root = root;
    if (check_open_vnode(root, node))
    {
        printf("File is already open.\n");
        return 0;
    }

    void *entry = entry_from_path(path, root);
    node->data = entry;
    if (is_directory(node, node->root->fs_type))
        node->type = VNODE_DIRECTORY;
    else
        node->type = VNODE_FILE;

    open_vnode(root, node);
    return node;
}

void fclose(vnode_t *node)
{
    vfs_root_t *root = get_root(node->path);
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

void fseek(vnode_t *node, unsigned long seek, enum FSeek_Types type)
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

unsigned long fread(void *buf, unsigned long size, vnode_t *node)
{
    if (is_directory(node, node->root->fs_type))
        return 0;

    if (node->root->fs_type == FAT32)
    {
        fatfs_node_t *entry = (fatfs_node_t *)node->data;
        fatfs_t *fs = (fatfs_t *)node->root->fs;
        if (size > entry->entry.size)
            size = entry->entry.size;

        void *init_buf = buf;
        unsigned long cl_size = fat_cluster_size(fs);
        unsigned long first_cluster = node->seek / cl_size;
        unsigned long last_cluster = (node->seek + size - 1) / cl_size;
        unsigned long start = node->seek % cl_size;
        unsigned long end = (node->seek + size) % cl_size;

        for (unsigned long cluster = first_cluster; cluster <= last_cluster; cluster++)
        {
            unsigned char *read = read_fatnode_at(*entry, cluster);
            unsigned char *cpy_start = read;
            unsigned long cpy_len = cl_size;

            if (cluster == first_cluster)
            {
                cpy_start += start;
                cpy_len -= start;
            }

            if (cluster == last_cluster)
                // edge case
                cpy_len = end ? end : cl_size;

            memcpy(buf, cpy_start, cpy_len);
            free(read);
            buf += cpy_len;
        }

        return buf - init_buf;
    }

    return 0;
}

void fwrite(void *buf, unsigned long size, vnode_t *node)
{
    if (is_directory(node, node->root->fs_type))
        return;

    if (node->root->fs_type == FAT32)
    {
        fatfs_node_t *entry = (fatfs_node_t *)node->data;
        write_to_fatnode(entry, buf, size);
    }
}
