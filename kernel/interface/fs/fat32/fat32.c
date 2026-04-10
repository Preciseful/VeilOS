/**
 * @author Developful
 * @brief VFS interface for the FAT32 filesystem.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/fat32/fat32.h>
#include <interface/errno.h>
#include <fs/fat32.h>
#include <lib/string.h>

bool FindParentFat32ByPath(FatFS *fs, const char *path, FatFSNode *buf)
{
    char *tmp = malloc(strlen(path) + 1);
    strcpy(tmp, path);

    char *slash = strrchr(tmp, '/');
    if (slash == tmp)
        slash[1] = '\0';
    else
        *slash = '\0';

    return FindFat32ByPath(fs, tmp, buf);
}

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

FilesystemInterface GetFat32Interface(FatFS *fatfs)
{
    FilesystemInterface interface;
    interface.fread = Fat32IRead;
    interface.fopen = Fat32IOpen;
    interface.fwrite = Fat32IWrite;
    interface.fcreate_directory = Fat32ICreateDirectory;
    interface.fcreate_file = Fat32ICreateFile;
    interface.fsize = Fat32IFileSize;
    interface.fpermissions = Fat32IPermissions;
    interface.key = fatfs;

    return interface;
}