#pragma once

#include <stdbool.h>
#include <interface/partition.h>

typedef struct __attribute__((packed)) Fat32ExtBS
{
    unsigned int table_size_32;
    unsigned short extended_flags;
    unsigned short fat_version;
    unsigned int root_cluster;
    unsigned short fat_info;
    unsigned short backup_BS_sector;
    unsigned char reserved_0[12];
    unsigned char drive_number;
    unsigned char reserved_1;
    unsigned char boot_signature;
    unsigned int volume_id;
    char volume_label[11];
    unsigned char fat_type_label[8];
} Fat32ExtBS;

typedef struct __attribute__((packed)) Fat32BS
{
    unsigned char bootjmp[3];
    unsigned char oem_name[8];
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sector_count;
    unsigned char table_count;
    unsigned short root_entry_count;
    unsigned short total_sectors_16;
    unsigned char media_type;
    unsigned short table_size_16;
    unsigned short sectors_per_track;
    unsigned short head_side_count;
    unsigned int hidden_sector_count;
    unsigned int total_sectors_32;
    struct Fat32ExtBS ext;
} Fat32BS;

typedef struct __attribute__((packed)) Fat32DirEntry
{
    unsigned char name[8];
    unsigned char ext[3];
    unsigned char attrs;
    unsigned char res;
    unsigned char create_ms;
    unsigned short create_time;
    unsigned short create_date;
    unsigned short access_date;
    unsigned short cluster_high;
    unsigned short modification_time;
    unsigned short modification_date;
    unsigned short cluster_low;
    unsigned int size;
} Fat32DirEntry;

typedef struct __attribute__((packed)) Fat32LFNEntry
{
    unsigned char order;
    unsigned short characters1[5];
    unsigned char attrs;
    unsigned char res1;
    unsigned char checksum;
    unsigned short characters2[6];
    unsigned short res2;
    unsigned short characters3[2];
} Fat32LFNEntry;

/**
 * @brief A wrapper around a FAT filesystem.
 * This keeps track of the partition it's in, along with other metadata.
 */
typedef struct FatFS
{
    Partition partition;
    Fat32BS *bs;
    unsigned int root_cluster;
    unsigned int first_data_sector;
    unsigned int first_fat_sector;
    unsigned int sectors_per_fat;
} FatFS;

/**
 * @brief A wrapper around a FAT directory entry.
 * This maintains the proper LFN name, along with its extension. It keeps track of the parent cluster and the filesystem it resides in.
 */
typedef struct FatFSNode
{
    unsigned int lfn_count;
    Fat32LFNEntry lfn_entries[20];
    Fat32DirEntry entry;
    char *name;
    char *extension;
    unsigned int cluster;
    unsigned int content_cluster;
    unsigned int parent_cluster;
    FatFS *fatfs;
} FatFSNode;

/**
 * @brief Initialize the FAT filesystem wrapper from a partition.
 *
 * @param[out] fs The filesystem wrapper.
 * @param partition The partition the FAT filesystem resides in.
 * @return `true` if it's a valid FAT filesystem, otherwise `false`.
 */
bool FatFSInit(FatFS *fs, Partition partition);

/**
 * @brief Get the cluster size from the filesystem.
 *
 * @param fs The filesystem wrapper.
 * @return The cluster size.
 */
unsigned int FatClusterSize(FatFS *fs);

/**
 * @brief Get the entries from a cluster.
 *
 * @param fs The filesystem wrapper.
 * @param cluster The cluster to obtain entries from.
 * @param[out] bnodes The entries array.
 * @return The count of entries.
 */
unsigned long GetFatEntries(FatFS *fs, unsigned int cluster, FatFSNode **bnodes);

/**
 * @brief Reads the entire contents of a file entry.
 *
 * @param node The entry to read from.
 * @return The contents.
 */
unsigned char *ReadFatNode(FatFSNode node);

/**
 * @brief Reads a certain amount of content of a file entry, starting from an offset.
 *
 * @param node The entry to read from.
 * @param offset The offset in file from which the read starts from.
 * @param size The amount of characters to read.
 * @return The contents.
 */
unsigned char *ReadFatNodeRange(FatFSNode node, unsigned int offset, unsigned int size);

/**
 * @brief Writes a buffer to a file entry.
 *
 * @param node The entry to write to.
 * @param cbuf The buffer.
 * @param size The size of the buffer.
 * @return `true` if the write was successful, otherwise `false`.
 */
bool WriteToFatNode(FatFSNode *node, const char *cbuf, unsigned long size);

/**
 * @brief Creates an entry in a directory.
 *
 * @param fs The filesystem wrapper.
 * @param parent_cluster The cluster of the parent directory.
 * @param name The name of the entry.
 * @param attrs The attributes of the new entry.
 * @return The entry created.
 */
FatFSNode CreateFatNode(FatFS *fs, unsigned int parent_cluster, const char *name, unsigned char attrs);