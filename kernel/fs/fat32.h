#pragma once

#include <stdbool.h>

typedef struct __attribute__((packed)) fat32_extbs
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
} fat32_extbs_t;

typedef struct __attribute__((packed)) fat32_bs
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
    struct fat32_extbs ext;
} fat32_bs_t;

typedef struct __attribute__((packed)) fat32_dir_entry
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
} fat32_entry_t;

typedef struct __attribute__((packed)) long_filename
{
    unsigned char order;
    unsigned short characters1[5];
    unsigned char attrs;
    unsigned char res1;
    unsigned char checksum;
    unsigned short characters2[6];
    unsigned short res2;
    unsigned short characters3[2];
} lfn_entry_t;

typedef struct fatfs
{
    fat32_bs_t *bs;
    unsigned int root_cluster;
    unsigned int first_data_sector;
    unsigned int first_fat_sector;
    unsigned int sectors_per_fat;
} fatfs_t;

typedef struct fatfs_node
{
    unsigned int lfn_count;
    lfn_entry_t lfn_entries[20];
    fat32_entry_t entry;
    char *name;
    char *extension;
    unsigned int cluster;
    unsigned int content_cluster;
    unsigned int parent_cluster;
    fatfs_t *fatfs;
} fatfs_node_t;

fatfs_t *fatfs_init();
unsigned int fat_cluster_size(fatfs_t *fs);
unsigned long get_fatentries(fatfs_t *fs, unsigned int cluster, fatfs_node_t **bnodes);

unsigned char *read_fatnode(fatfs_node_t node);
unsigned char *read_fatnode_at(fatfs_node_t node, unsigned long pos);

bool write_to_fatnode(fatfs_node_t *node, const char *cbuf, unsigned long size);
fatfs_node_t create_fatnode(fatfs_t *fs, unsigned int parent_cluster, const char *name, unsigned char attrs);