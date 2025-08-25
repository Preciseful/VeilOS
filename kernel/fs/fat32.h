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

typedef struct FatFS
{
    Partition partition;
    Fat32BS *bs;
    unsigned int root_cluster;
    unsigned int first_data_sector;
    unsigned int first_fat_sector;
    unsigned int sectors_per_fat;
} FatFS;

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

FatFS *FatFSInit(Partition partition);
unsigned int FatClusterSize(FatFS *fs);
unsigned long GetFatEntries(FatFS *fs, unsigned int cluster, FatFSNode **bnodes);

unsigned char *ReadFatNode(FatFSNode node);
unsigned char *ReadFatNodeAt(FatFSNode node, unsigned long pos);

bool WriteToFatNode(FatFSNode *node, const char *cbuf, unsigned long size);
FatFSNode CreateFatNode(FatFS *fs, unsigned int parent_cluster, const char *name, unsigned char attrs);