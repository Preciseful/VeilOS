#include <fs/fat32.hpp>
#include <drivers/emmc.h>
#include <lib/printf.h>
#include <fs/mbr.hpp>
#include <mm.h>
#include <lib/string.h>

using namespace veil::fs;
using namespace veil::std;

unsigned char FAT32LongFileNameEntry::GetCountFrom(unsigned long index)
{
    return this->internals[index]->order & 0x3F;
}

bool FAT32LongFileNameEntry::GetFinishingBit(unsigned long index)
{
    return this->internals[index]->order & 0x40;
}

bool FAT32LongFileNameEntry::GetDeletedBit(unsigned long index)
{
    return this->internals[index]->order & 0x80;
}

unsigned char get_char_from_short(unsigned short s)
{
    unsigned char lsb = s & 0xFF;
    unsigned char msb = (s >> 8) & 0xFF;
    if (msb != 0)
        return '?';
    return lsb;
}

unsigned char *FAT32LongFileNameEntry::GetNormalStringFrom(unsigned long index)
{
    unsigned char *name = new unsigned char[27];
    struct long_filename *internal = this->internals[index];

    int name_index = 0;

    for (int i = 0; i < 5; i++, name_index++)
        name[name_index] = get_char_from_short(internal->characters1[i]);
    for (int i = 0; i < 6; i++, name_index++)
        name[name_index] = get_char_from_short(internal->characters2[i]);
    for (int i = 0; i < 2; i++, name_index++)
        name[name_index] = get_char_from_short(internal->characters3[i]);

    name[name_index] = '\0';
    return name;
}

FAT32DirectoryEntry::FAT32DirectoryEntry(struct fat32_dir_entry *entry)
    : internal(entry)
{
    this->name = new unsigned char[256];
    memcpy(this->name, internal->name, 8);
    this->name[9] = 0;

    this->extension = internal->ext;
    this->cluster = (internal->cluster_high << 16) | internal->cluster_low;
}

void FAT32DirectoryEntry::SetFilename(FAT32LongFileNameEntry filename)
{
    this->filename = filename;
    // use i-- > 0 because unsigned longs wrap around so we cant do i >= 0
    unsigned long count = 0;

    for (unsigned long i = filename.internals.Count(); i-- > 0;)
    {
        unsigned char *name = filename.GetNormalStringFrom(i);
        unsigned long len = strlen(name);

        memcpy(&this->name[count], name, len);
        count += len;

        delete[] name;
    }
}

struct fat32_bs *find_fat(unsigned int offset)
{
    emmc_seek(offset);
    unsigned char *buf = new unsigned char[512];
    int read = emmc_read(buf, 512);

    if (read < 0)
    {
        printf("Read failed.\n");
        return 0;
    }

    if (read != 512)
    {
        printf("Read only %d bytes.\n", read);
        return 0;
    }

    struct fat32_bs *bs = (struct fat32_bs *)buf;
    if (bs->bootjmp[0] != 0xeb)
    {
        printf("Not a valid FAT filesystem.\n", bs->bootjmp[0]);
        return 0;
    }

    unsigned int root_dir_sectors = (bs->root_entry_count * 32 + bs->bytes_per_sector - 1) / bs->bytes_per_sector;
    if (root_dir_sectors != 0)
    {
        printf("Root dir sectors isnt 0. FAT32 requires this to be 0.\n");
        return 0;
    }

    return bs;
}

FatFS::FatFS()
{
    this->succeded = false;

    struct master_boot_record mbr;
    emmc_read((unsigned char *)&mbr, sizeof(mbr));

    if (mbr.bootSignature != BOOT_SIGNATURE)
    {
        printf("Bad MBR signature.");
        return;
    }

    // search first FAT fs
    for (int i = 0; mbr.partitions[i].type != 0; i++)
    {
        this->bs = find_fat(mbr.partitions[i].first_lba_sector * 512);
        if (this->bs)
            break;
    }

    if (!this->bs)
        return;

    this->root_cluster = bs->ext.root_cluster;
    this->first_data_sector = bs->reserved_sector_count + (bs->table_count * bs->ext.table_size_32) + 8192;
    this->first_fat_sector = bs->reserved_sector_count + 8192;
    this->sectors_per_fat = bs->ext.table_size_32;

    this->succeded = true;
}

unsigned int FatFS::nextCluster(unsigned int cluster_no)
{
    unsigned char FAT_table[512];

    unsigned int fat_offset = cluster_no * 4;
    unsigned int fat_sector = first_fat_sector + (fat_offset / bs->bytes_per_sector);
    unsigned int ent_offset = fat_offset % bs->bytes_per_sector;

    // 8192 again being hard value
    emmc_seek(fat_sector * 512);
    emmc_read(FAT_table, 512);

    unsigned int table_value = *(unsigned int *)&FAT_table[ent_offset];
    table_value &= 0x0fffffff;

    // end
    if (table_value >= 0x0FFFFFF8)
        return 0;
    return table_value;
}

unsigned int FatFS::readBytesFromCluster(unsigned int cluster, unsigned char *&buf)
{
    unsigned int cluster_sector = ((cluster - 2) * bs->sectors_per_cluster) + first_data_sector;
    unsigned int entries_bytes = bs->sectors_per_cluster * bs->bytes_per_sector;
    buf = new unsigned char[entries_bytes];

    emmc_seek(cluster_sector * 512);
    emmc_read(buf, entries_bytes);

    return entries_bytes;
}

FAT32DirectoryEntry FatFS::GetEntry(unsigned int cluster)
{
    List<struct long_filename *> filenames;

    while (true)
    {
        unsigned char *buf;
        unsigned int entries_bytes = readBytesFromCluster(cluster, buf);

        for (unsigned int i = 0; i < entries_bytes; i += 32)
        {
            if (buf[i] == 0x0)
                return 0;

            if (buf[i] == 0xE5)
                continue;

            if (buf[i + 11] == 0x0F)
            {
                struct long_filename *filename = (struct long_filename *)&buf[i];
                filenames.Add(filename);
                continue;
            }

            struct fat32_dir_entry *entry = (struct fat32_dir_entry *)&buf[i];
            FAT32DirectoryEntry directory(entry);

            if (filenames.Count() > 0)
            {
                FAT32LongFileNameEntry entry(filenames);
                directory.SetFilename(entry);

                filenames.Clear();
            }

            return directory;
        }
    }
}

List<FAT32DirectoryEntry> FatFS::GetEntries(unsigned int cluster)
{
    List<FAT32DirectoryEntry> entries;
    List<struct long_filename *> filenames;

    while (true)
    {
        unsigned char *buf;
        unsigned int entries_bytes = readBytesFromCluster(cluster, buf);

        for (unsigned int i = 0; i < entries_bytes; i += 32)
        {
            if (buf[i] == 0x0)
                return entries;

            if (buf[i] == 0xE5)
                continue;

            if (buf[i + 11] == 0x0F)
            {
                struct long_filename *filename = (struct long_filename *)&buf[i];
                filenames.Add(filename);
                continue;
            }

            struct fat32_dir_entry *entry = (struct fat32_dir_entry *)&buf[i];
            FAT32DirectoryEntry directory(entry);

            if (filenames.Count() > 0)
            {
                FAT32LongFileNameEntry entry(filenames);
                directory.SetFilename(entry);

                filenames.Clear();
            }

            entries.Add(directory);
        }

        cluster = nextCluster(cluster);
        if (cluster == 0)
            return entries;
    }
}

unsigned char *FatFS::ReadFile(FAT32DirectoryEntry *entry)
{
    unsigned int cluster = entry->cluster;
    unsigned char *file = new unsigned char[entry->Size()];
    unsigned char *buf;
    unsigned int count = 0;

    while (true)
    {
        unsigned int bytes = readBytesFromCluster(cluster, buf);

        for (unsigned int i = 0; i < bytes; i++)
            file[count + i] = buf[i];

        count += bytes;

        cluster = nextCluster(cluster);
        if (cluster == 0)
            break;
    }

    return file;
}
