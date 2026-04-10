#include <interface/fs/fat32/fat32.h>
#include <interface/errno.h>
#include <fs/fat32.h>
#include <lib/string.h>

bool FindFat32ByPath(FatFS *fs, const char *path, FatFSNode *buf)
{
    if (path[0] != '/')
        return false;

    if (strcmp(path, "/") == 0)
    {
        FatFSNode *root_nodes;
        GetFatEntries(fs, fs->root_cluster, &root_nodes);

        memcpy(buf, &root_nodes[0], sizeof(FatFSNode));

        free(root_nodes);
        return true;
    }

    FatFSNode node;
    unsigned long cluster = fs->root_cluster;
    const char *beginning = path + 1;

    while (true)
    {
        const char *end = beginning;
        while (*end != '\0' && *end != '/')
            end++;

        unsigned long filename_len = end - beginning;

        char *filename = malloc(filename_len + 1);
        memcpy(filename, beginning, filename_len);
        filename[filename_len] = 0;

        FatFSNode *nodes;
        unsigned long nodes_count = GetFatEntries(fs, cluster, &nodes);
        unsigned long index = 0;

        for (index = 0; index < nodes_count; index++)
        {
            if (strcmp(nodes[index].name, filename) == 0)
                break;
        }

        if (index == nodes_count)
        {
            free(filename);
            free(nodes);
            return false;
        }

        node = nodes[index];
        free(nodes);
        free(filename);

        if (*end == '\0')
            break;

        beginning = end + 1;
        cluster = node.cluster;
    }

    if (buf != 0)
        memcpy(buf, &node, sizeof(FatFSNode));
    return true;
}

long Fat32IRead(const char *path, enum File_Mode mode, char *buf, unsigned long size, unsigned long offset, void *key)
{
    FatFSNode node;
    FatFS *fs = (FatFS *)key;

    if (!FindFat32ByPath(fs, path, &node))
        return -E_NO_FILE;

    unsigned char *tempbuf = ReadFatNodeRange(node, offset, size);
    memcpy(buf, tempbuf, size);

    free(tempbuf);

    return size;
}