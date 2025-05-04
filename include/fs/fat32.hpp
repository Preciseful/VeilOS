#ifndef FAT32_HPP
#define FAT32_HPP

#include <stdbool.h>
#include <lib/list.hpp>

namespace veil
{
    class City;
}

namespace veil
{
    struct fat32_extbs
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
    } __attribute__((packed));

    struct fat32_bs
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
    } __attribute__((packed));

    struct fat32_dir_entry
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
    } __attribute__((packed));

    struct long_filename
    {
        unsigned char order;
        unsigned short characters1[5];
        unsigned char attrs;
        unsigned char res1;
        unsigned char checksum;
        unsigned short characters2[6];
        unsigned short res2;
        unsigned short characters3[2];
    } __attribute__((packed));

    class FAT32LongFileNameEntry
    {
    public:
        veil::std::List<struct long_filename *> internals;

        unsigned char GetCountFrom(unsigned long index);
        unsigned char *GetNormalStringFrom(unsigned long index);
        bool GetFinishingBit(unsigned long index);
        bool GetDeletedBit(unsigned long index);

        FAT32LongFileNameEntry(veil::std::List<struct long_filename *> internals)
            : internals(internals) {}
        FAT32LongFileNameEntry() {}
    };

    class FAT32DirectoryEntry
    {
        FAT32LongFileNameEntry filename;

    public:
        unsigned long lfn_count = 0;
        unsigned char *name;
        unsigned char *extension;
        bool filename_set = false;
        unsigned int cluster;
        unsigned int parent_cluster = 0;
        struct fat32_dir_entry *internal;

        void SetFilename(FAT32LongFileNameEntry filename);

        unsigned char Attributes()
        {
            return internal->attrs;
        }

        unsigned int Size()
        {
            return internal->size;
        }

        FAT32DirectoryEntry(struct fat32_dir_entry *entry);
    };

    class FatFS
    {
        struct fat32_bs *bs;
        unsigned int first_data_sector;
        unsigned int first_fat_sector;
        unsigned int sectors_per_fat;

        unsigned int nextCluster(unsigned int cluster_no);
        unsigned int readBytesFromCluster(unsigned int cluster, unsigned char *&buf);
        unsigned int writeBytesToCluster(unsigned int cluster, unsigned char *buf);
        unsigned int findFreeCluster();
        unsigned int linkFreeCluster(unsigned int cluster);
        void writeToEntry(unsigned int cluster_no, unsigned int value);
        bool updateDirectoryEntry(unsigned int parent_cluster, FAT32DirectoryEntry *target_entry, bool clean_clusters);

    public:
        bool succeded;
        unsigned int root_cluster;
        FAT32DirectoryEntry WriteNewEntry(City *parent_city, unsigned int parent_cluster, const char *name, unsigned char attrs);
        FAT32DirectoryEntry MoveEntry(City *parent_city, FAT32DirectoryEntry entry, const char *name);
        veil::std::List<FAT32DirectoryEntry> GetEntries(unsigned int cluster);
        FAT32DirectoryEntry GetEntry(unsigned int cluster);
        unsigned char *ReadFile(FAT32DirectoryEntry *entry);
        bool WriteToEntry(FAT32DirectoryEntry *entry, const char *cbuf, unsigned long size);
        bool DeleteEntry(FAT32DirectoryEntry *entry, bool keep_data);
        FatFS();
    };
}

#endif