#include <vfs/vnode.h>
#include <lib/string.h>
#include <memory/memory.h>
#include <vfs/vfs.h>
#include <fs/fat32.h>
#include <lib/printf.h>

vfs_root_t *get_root(const char *path)
{
    for (unsigned long i = 0; i < vfs->roots_count; i++)
    {
        if (path[0] == vfs->roots[i].id)
            return &vfs->roots[i];
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

fatfs_node_t *fat_entry_from_path(char *path, vfs_root_t *root)
{
    bool found_node = false;
    bool finishing = false;
    fatfs_node_t *node = malloc(sizeof(fatfs_node_t));
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

        fatfs_node_t *nodes;
        unsigned long nodes_count = get_fatentries(root->fs, !found_node ? ((fatfs_t *)root->fs)->root_cluster : node->content_cluster, &nodes);
        found_node = false;

        for (unsigned long i = 0; i < nodes_count; i++)
        {
            if (strlen(nodes[i].name) != entry_path_size)
                continue;
            if (memcmp(nodes[i].name, &path[start], entry_path_size) != 0)
                continue;

            memcpy(node, &nodes[i], sizeof(fatfs_node_t));
            found_node = true;
            break;
        }

        free(nodes);

        if (!found_node)
        {
            printf("Cannot find: \"");
            for (unsigned long i = 0; i < entry_path_size; i++)
            {
                printf("%c", path[start + i]);
            }

            printf("\"\n");
            return 0;
        }

        if (!finishing && !(node->entry.attrs & 0x10))
        {
            printf("File used as a directory.\n");
            return 0;
        }

        if (finishing && node->entry.attrs & 0x10)
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

vnode_t *fopen(char *path)
{
    unsigned long len = strlen(path);
    if (len == 0)
        return 0;

    vfs_root_t *root = get_root(path);
    if (!root)
        return 0;

    vnode_t *node = malloc(sizeof(vnode_t));
    node->path = path;
    node->fs_type = root->fs_type;
    if (check_open_vnode(root, node))
    {
        printf("File is already open.\n");
        return 0;
    }

    if (node->fs_type == FAT32)
    {
        fatfs_node_t *entry = fat_entry_from_path(path, root);
        node->data = entry;
        if (entry->entry.attrs & 0x10)
            node->type = VNODE_DIRECTORY;
        else
            node->type = VNODE_FILE;
    }

    open_vnode(root, node);
    return node;
}