#include <fs/fat32.hpp>
#include <drivers/emmc.h>
#include <lib/printf.h>
#include <fs/mbr.hpp>
#include <mm.h>
#include <lib/string.h>
#include <fs/vfs/file.hpp>
#include <fs/vfs/city.hpp>

using namespace veil;
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
        ERROR("Read failed.\n");
        return 0;
    }

    if (read != 512)
    {
        ERROR("Read only %d bytes.\n", read);
        return 0;
    }

    struct fat32_bs *bs = (struct fat32_bs *)buf;
    if (bs->bootjmp[0] != 0xeb)
    {
        ERROR("Not a valid FAT filesystem.\n", bs->bootjmp[0]);
        return 0;
    }

    unsigned int root_dir_sectors = (bs->root_entry_count * 32 + bs->bytes_per_sector - 1) / bs->bytes_per_sector;
    if (root_dir_sectors != 0)
    {
        ERROR("Root dir sectors isnt 0. FAT32 requires this to be 0.\n");
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
        ERROR("Bad MBR signature.");
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

    emmc_seek(fat_sector * 512);
    emmc_read(FAT_table, 512);

    unsigned int table_value = *(unsigned int *)&FAT_table[ent_offset];
    table_value &= 0x0fffffff;

    // end
    if (table_value >= 0x0FFFFFF8)
        return 0;
    return table_value;
}

unsigned int FatFS::findFreeCluster()
{
    unsigned char FAT_table[512];
    unsigned int entries = bs->bytes_per_sector / 4;

    for (unsigned int i = 0; i < sectors_per_fat; i++)
    {
        emmc_seek((first_fat_sector + i) * 512);
        emmc_read(FAT_table, 512);

        for (unsigned int entry = 0; entry < entries; entry++)
        {
            unsigned int value = *(unsigned int *)&FAT_table[entry * 4];
            value &= 0x0FFFFFFF;

            if (value == 0x00000000)
            {
                unsigned int cluster = (i * entries) + entry;

                if (cluster >= 2)
                    return cluster;
            }
        }
    }

    return 0;
}

void FatFS::writeToEntry(unsigned int cluster_no, unsigned int value)
{
    unsigned char FAT_table[512];
    unsigned int fat_offset = cluster_no * 4;
    unsigned int fat_sector = first_fat_sector + (fat_offset / bs->bytes_per_sector);
    unsigned int ent_offset = fat_offset % bs->bytes_per_sector;

    emmc_seek(fat_sector * 512);
    emmc_read(FAT_table, 512);

    value &= 0x0FFFFFFF;
    *(unsigned int *)&FAT_table[ent_offset] = value;

    emmc_seek(fat_sector * 512);
    emmc_write(FAT_table, 512);
}

unsigned int FatFS::linkFreeCluster(unsigned int cluster)
{
    unsigned int free = findFreeCluster();
    unsigned int last = cluster;
    while (true)
    {
        unsigned int next = nextCluster(last);
        if (!next)
            break;
        last = next;
    }

    writeToEntry(last, free);
    writeToEntry(free, 0x0FFFFFFF);
    return free;
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

unsigned int FatFS::writeBytesToCluster(unsigned int cluster, unsigned char *buf)
{
    unsigned int cluster_sector = ((cluster - 2) * bs->sectors_per_cluster) + first_data_sector;
    unsigned int entries_bytes = bs->sectors_per_cluster * bs->bytes_per_sector;

    emmc_seek(cluster_sector * 512);
    emmc_write(buf, entries_bytes);

    return entries_bytes;
}

bool FatFS::updateDirectoryEntry(unsigned int parent_cluster, FAT32DirectoryEntry *target_entry, bool clean_clusters)
{
    unsigned int cluster_sector = (parent_cluster - 2) * bs->sectors_per_cluster + first_data_sector;

    for (unsigned int sector = 0; sector < bs->sectors_per_cluster; sector++)
    {
        unsigned char sector_buf[512];
        emmc_seek((cluster_sector + sector) * 512);
        emmc_read(sector_buf, 512);

        fat32_dir_entry *entries = (fat32_dir_entry *)sector_buf;
        for (unsigned int i = 0; i < 16; i++)
        {
            unsigned char x[12];
            memcpy(x, entries[i].name, 11);
            x[11] = '\0';

            unsigned char y[12];
            memcpy(y, target_entry->internal->name, 11);
            y[11] = '\0';

            if (strcmp(x, y) == 0)
            {
                memcpy(&entries[i], target_entry->internal, sizeof(fat32_dir_entry));

                unsigned int content = target_entry->cluster;
                unsigned int entries_bytes = bs->sectors_per_cluster * bs->bytes_per_sector;
                unsigned char clean[entries_bytes];
                memset(clean, 0, entries_bytes);

                while (content != 0 && clean_clusters)
                {
                    writeBytesToCluster(content, clean);
                    content = nextCluster(content);
                }

                emmc_seek((cluster_sector + sector) * 512);
                emmc_write(sector_buf, 512);
                return true;
            }
        }
    }

    unsigned int next_cluster = nextCluster(parent_cluster);
    if (next_cluster != 0)
        return updateDirectoryEntry(next_cluster, target_entry, clean_clusters);

    return false;
}

unsigned char lfn_checksum(const unsigned char *short_name)
{
    unsigned char sum = 0;
    for (int i = 11; i != 0; i--)
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *short_name++;
    return sum;
}

unsigned char *generate_short(const char *name)
{
    unsigned char *short_name = (unsigned char *)valloc(11);
    memset(short_name, ' ', 11);
    int dot = 0;

    for (unsigned long i = 0; i < strlen((unsigned char *)name); i++)
    {
        if (name[i] == '.')
        {
            dot = i;
            break;
        }
    }

    for (int i = 0; i < 8; i++)
    {
        if (name[i] == '.')
            break;

        short_name[i] = toupper(name[i]);
    }

    for (int i = dot + 1; i < dot + 4 && name[i]; i++)
        short_name[(i - dot - 1) + 8] = toupper(name[i]);

    return short_name;
}

FAT32DirectoryEntry FatFS::WriteNewEntry(City *parent_city, unsigned int parent_cluster, const char *name, unsigned char attrs)
{
    unsigned char *short_name = generate_short(name);
    unsigned long namelen = strlen((const unsigned char *)name);
    unsigned char checksum = lfn_checksum(short_name);

    while (true)
    {
        unsigned char *buf;
        unsigned int entries_bytes = readBytesFromCluster(parent_cluster, buf);
        bool null_terminated = false;

        for (unsigned int i = 0; i < entries_bytes; i += 32)
        {
            if (buf[i] != 0x0 && buf[i] != 0xE5)
                continue;

            unsigned int lfn_count = (namelen + 12) / 13;
            bool ok = true;
            for (unsigned int j = 0; j < lfn_count + 1; j++)
            {
                if ((buf[i + j * 32] != 0x0 && buf[i + j * 32] != 0xE5) || i + j * 32 >= entries_bytes)
                {
                    ok = false;
                    break;
                }
            }

            if (!ok)
                continue;

            for (unsigned int j = 0; j < lfn_count; j++)
            {
                struct long_filename *lfn = (struct long_filename *)&buf[i + (lfn_count - j - 1) * 32];
                memzero((unsigned long)lfn, sizeof(struct long_filename));

                lfn->checksum = checksum;
                lfn->order = (j + 1);
                if (j == lfn_count - 1)
                    lfn->order |= 0x40;
                lfn->attrs = 0x0F;
                lfn->res1 = 0x0000;
                lfn->res2 = 0x0000;

                unsigned int name_offset = j * 13;

                for (int k = 0; k < 5; k++)
                {
                    if (name_offset + k < namelen)
                        lfn->characters1[k] = (unsigned short)name[name_offset + k];
                    else if (name_offset + k == namelen && !null_terminated)
                    {
                        lfn->characters1[k] = '\0';
                        null_terminated = true;
                    }
                    else
                        lfn->characters1[k] = 0xFFFF;
                }

                for (int k = 0; k < 6; k++)
                {
                    if (name_offset + 5 + k < namelen)
                        lfn->characters2[k] = (unsigned short)name[name_offset + 5 + k];
                    else if (name_offset + 5 + k == namelen && !null_terminated)
                    {
                        lfn->characters2[k] = '\0';
                        null_terminated = true;
                    }
                    else
                        lfn->characters2[k] = 0xFFFF;
                }

                for (int k = 0; k < 2; k++)
                {
                    if (name_offset + 11 + k < namelen)
                        lfn->characters3[k] = (unsigned short)name[name_offset + 11 + k];
                    else if (name_offset + 11 + k == namelen && !null_terminated)
                    {
                        lfn->characters3[k] = '\0';
                        null_terminated = true;
                    }
                    else
                        lfn->characters3[k] = 0xFFFF;
                }
            }

            unsigned int offset = i + lfn_count * 32;
            struct fat32_dir_entry *dir = (struct fat32_dir_entry *)&buf[offset];

            unsigned int cluster = 0;

            memzero((unsigned long)dir, sizeof(struct fat32_dir_entry));
            memcpy(dir->name, short_name, 8);
            memcpy(dir->ext, short_name + 8, 3);

            dir->attrs = attrs;
            dir->cluster_high = (cluster >> 16) & 0xFFFF;
            dir->cluster_low = cluster & 0xFFFF;
            dir->size = 0;

            dir->create_time = 0x0000;
            dir->create_date = 0x28C0;
            dir->modification_time = 0x0000;
            dir->modification_date = 0x28C0;
            dir->access_date = 0x28C0;

            unsigned int cluster_sector = ((parent_cluster - 2) * bs->sectors_per_cluster) + first_data_sector;

            emmc_seek(cluster_sector * 512);
            emmc_write(buf, entries_bytes);
            INFO("Created file %s.\n", name);

            delete[] short_name;
            auto finalent = FAT32DirectoryEntry(dir);
            City *city = new City(parent_city, name, FileType, finalent, this);
            parent_city->AddSubcity(city);
            finalent.parent_cluster = parent_cluster;
            finalent.lfn_count = lfn_count;

            return finalent;
        }

        unsigned int next = nextCluster(parent_cluster);
        if (next == 0)
        {
            next = linkFreeCluster(parent_cluster);
        }

        parent_cluster = next;
    }
}

FAT32DirectoryEntry FatFS::MoveEntry(City *parent_city, FAT32DirectoryEntry entry, const char *name)
{
    FAT32DirectoryEntry copy = entry;
    DeleteEntry(&entry, false);
    entry = WriteNewEntry(parent_city, parent_city->GetCluster(), name, copy.internal->attrs);

    entry.internal->size = copy.internal->size;
    entry.internal->cluster_high = copy.internal->cluster_high;
    entry.internal->cluster_low = copy.internal->cluster_low;
    entry.cluster = copy.cluster;
    entry.parent_cluster = copy.parent_cluster;

    updateDirectoryEntry(parent_city->GetCluster(), &entry, false);
    return entry;
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
                directory.lfn_count = filenames.Count();
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
                directory.lfn_count = filenames.Count();
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

bool FatFS::WriteToEntry(FAT32DirectoryEntry *entry, const char *cbuf, unsigned long size)
{
    unsigned char *buf = new unsigned char[size];
    memcpy(buf, cbuf, size);

    unsigned int cluster = entry->cluster;
    if (cluster == 0)
    {
        cluster = findFreeCluster();
        writeToEntry(cluster, 0x0FFFFFFF);
        entry->cluster = cluster;
        entry->internal->cluster_low = cluster & 0xFFFF;
        entry->internal->cluster_high = (cluster >> 16) & 0xFFFF;
    }

    unsigned long padded_size = ((size + 511) / 512) * 512;
    unsigned char *padded_buf = nullptr;
    if (size % 512 != 0)
    {
        padded_buf = new unsigned char[padded_size];
        memset(padded_buf, 0, padded_size);
        memcpy(padded_buf, buf, size);
        buf = padded_buf;
    }

    for (unsigned long i = 0; i < padded_size; i += 512)
    {
        writeBytesToCluster(cluster, buf + i);
        unsigned int next = nextCluster(cluster);
        if (next == 0)
            next = linkFreeCluster(cluster);

        cluster = next;
    }

    entry->internal->size = size;
    updateDirectoryEntry(entry->parent_cluster, entry, false);

    delete[] buf;
    if (padded_buf)
        delete[] padded_buf;

    return true;
}

bool FatFS::DeleteEntry(FAT32DirectoryEntry *entry, bool clean)
{
    unsigned int cluster = entry->cluster;
    while (cluster != 0 && clean)
    {
        writeToEntry(cluster, 0x00000000);
        cluster = nextCluster(cluster);
    }

    updateDirectoryEntry(entry->parent_cluster, entry, clean);
    unsigned int parent_cluster = entry->parent_cluster;
    while (parent_cluster >= 2)
    {
        unsigned int cluster_sector = (parent_cluster - 2) * bs->sectors_per_cluster + first_data_sector;

        for (unsigned int sector = 0; sector < bs->sectors_per_cluster; ++sector)
        {
            unsigned char sector_buf[512];
            emmc_seek((cluster_sector + sector) * 512);
            emmc_read(sector_buf, 512);

            fat32_dir_entry *entries = (struct fat32_dir_entry *)(sector_buf);
            for (unsigned long i = 0; i < 512 / sizeof(fat32_dir_entry); i++)
            {
                if (memcmp(entries[i].name, entry->internal->name, 11) == 0)
                {
                    entries[i].name[0] = 0xE5;

                    for (int j = i - 1; j >= 0; --j)
                    {
                        if (entries[j].attrs == 0x0F)
                            entries[j].name[0] = 0xE5;
                        else
                            break;
                    }

                    emmc_seek((cluster_sector + sector) * 512);
                    emmc_write(sector_buf, 512);
                    entry->internal->name[0] = 0xE5;
                    updateDirectoryEntry(entry->parent_cluster, entry, clean);
                    return true;
                }
            }
        }

        parent_cluster = nextCluster(parent_cluster);
    }

    return false;
}