/**
 * @author Developful
 * @brief VFS FAT32 filesystem writing functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fs/fat32/fat32.h>
#include <interface/errno.h>
#include <fs/fat32.h>
#include <lib/string.h>

long Fat32IWrite(const char *path, const char *buf, unsigned long size, unsigned long offset, void *file, void *key)
{
    FatFSNode node;
    FatFS *fatfs = (FatFS *)key;

    if (!FindFat32ByPath(fatfs, path, &node))
        return -E_NO_FILE;

    unsigned int cluster = node.content_cluster;

    if (cluster == 0)
    {
        cluster = FreeCluster(node.fatfs);
        WriteClusterLink(node.fatfs, cluster, 0x0FFFFFFF);

        node.content_cluster = cluster;
        node.entry.cluster_low = cluster & 0xFFFF;
        node.entry.cluster_high = (cluster >> 16) & 0xFFFF;
    }

    unsigned long cluster_size = FatClusterSize(fatfs);

    unsigned long cluster_index = offset / cluster_size;
    unsigned long cluster_offset = offset % cluster_size;

    for (unsigned long i = 0; i < cluster_index; i++)
    {
        unsigned int next = NextCluster(node.fatfs, cluster);
        if (next == 0)
            next = LinkFreeCluster(node.fatfs, cluster);

        cluster = next;
    }

    unsigned long written = 0;
    while (written < size)
    {
        unsigned char tmp[FatClusterSize(fatfs)];

        ReadCluster(node.fatfs, cluster, tmp);

        unsigned long write_start = (written == 0) ? cluster_offset : 0;
        unsigned long write_len = FatClusterSize(fatfs) - write_start;

        if (write_len > size - written)
            write_len = size - written;

        memcpy(tmp + write_start, buf + written, write_len);
        WriteCluster(node.fatfs, cluster, tmp);
        written += write_len;

        if (written < size)
        {
            unsigned int next = NextCluster(node.fatfs, cluster);
            if (next == 0)
                next = LinkFreeCluster(node.fatfs, cluster);

            cluster = next;
        }
    }

    if (offset + size > node.entry.size)
        node.entry.size = offset + size;

    UpdateFatEntry(node.parent_cluster, &node, false);
    return written;
}