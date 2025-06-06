#include <fs/fat32.h>
#include <fs/mbr.h>
#include <drivers/emmc.h>
#include <memory/memory.h>
#include <lib/printf.h>
#include <lib/string.h>

unsigned int fat_cluster_size(fatfs_t *fs)
{
    return fs->bs->sectors_per_cluster * fs->bs->bytes_per_sector;
}

unsigned char get_char_from_short(unsigned short s)
{
    unsigned char lsb = s & 0xFF;
    unsigned char msb = (s >> 8) & 0xFF;
    if (msb != 0)
        return '?';
    return lsb;
}

unsigned char *utf8_filename(lfn_entry_t entry)
{
    int name_index = 0;
    unsigned char *name = malloc(27);
    memset(name, 0, 13);

    for (int i = 0; i < 5; i++, name_index++)
        name[name_index] = get_char_from_short(entry.characters1[i]);
    for (int i = 0; i < 6; i++, name_index++)
        name[name_index] = get_char_from_short(entry.characters2[i]);
    for (int i = 0; i < 2; i++, name_index++)
        name[name_index] = get_char_from_short(entry.characters3[i]);

    name[name_index] = '\0';
    return name;
}

unsigned long last_nonspace(unsigned long size, unsigned char *s)
{
    unsigned long lasti = 0;
    for (unsigned long i = size; i-- > 0;)
    {
        if (s[i] != ' ')
        {
            lasti = i;
            break;
        }
    }

    return lasti;
}

void set_basic_filename(fatfs_t *fs, fatfs_node_t *node)
{
    unsigned long namespace = last_nonspace(8, node->entry.name) + 1;
    unsigned long extspace = last_nonspace(3, node->entry.ext) + 1;
    node->name = malloc(namespace + extspace + 1);
    memcpy(node->name, node->entry.name, namespace);

    if (extspace == 0)
        node->name[namespace] = '\0';
    else
    {
        node->name[namespace++] = '.';
        memcpy(node->name + namespace, node->entry.ext, extspace);
        node->name[namespace + extspace] = '\0';
    }
}

void set_filename(fatfs_t *fs, fatfs_node_t *node)
{
    unsigned int count = node->lfn_count;
    if (count == 0)
    {
        set_basic_filename(fs, node);
        return;
    }

    node->name = malloc(count * 26 + 1);
    unsigned int j = 0;

    for (unsigned int i = count; i-- > 0;)
    {
        unsigned char *name = utf8_filename(node->lfn_entries[i]);
        unsigned long len = ustrlen(name);

        memcpy(&node->name[j], name, len);
        j += len;

        free(name);
    }

    unsigned long exti = 0;
    bool foundext = false;
    for (unsigned long i = 0; i < strlen(node->name); i++)
    {
        if (node->name[i] == '.')
        {
            exti = i;
            foundext = true;
            // dont break because we want last . occurence
        }
    }

    if (!foundext)
    {
        node->extension = malloc(4);
        memcpy(node->extension, node->entry.ext, 3);
        node->extension[3] = '\0';
    }
    else
    {
        unsigned long extlen = strlen(&node->name[exti]) + 1;
        node->extension = malloc(extlen);
        memcpy(node->extension, &node->name[exti], extlen);
    }
}

fat32_bs_t *get_fat(unsigned int offset)
{
    emmc_seek(offset);
    unsigned char *buf = malloc(512);
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

    fat32_bs_t *bs = (fat32_bs_t *)buf;
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

fatfs_t *fatfs_init()
{
    fatfs_t *fs = malloc(sizeof(fatfs_t));
    fat32_bs_t *bs;
    mbr_t mbr;
    emmc_read((unsigned char *)&mbr, sizeof(mbr));

    if (mbr.bootSignature != BOOT_SIGNATURE)
    {
        printf("Bad MBR signature.\n");
        return 0;
    }

    for (int i = 0; mbr.partitions[i].type != 0; i++)
    {
        bs = get_fat(mbr.partitions[i].first_lba_sector * 512);
        if (bs)
            break;
    }

    if (!bs)
        return 0;

    fs->bs = bs;
    fs->root_cluster = bs->ext.root_cluster;
    fs->first_data_sector = bs->reserved_sector_count + (bs->table_count * bs->ext.table_size_32) + 8192;
    fs->first_fat_sector = bs->reserved_sector_count + 8192;
    fs->sectors_per_fat = bs->ext.table_size_32;
    return fs;
}

unsigned int free_cluster(fatfs_t *fs)
{
    unsigned char FAT_table[512];
    unsigned int entries = fs->bs->bytes_per_sector / 4;

    for (unsigned int i = 0; i < fs->sectors_per_fat; i++)
    {
        emmc_seek((fs->first_fat_sector + i) * 512);
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

unsigned int next_cluster(fatfs_t *fs, unsigned int cluster_no)
{
    unsigned char FAT_table[512];

    unsigned int fat_offset = cluster_no * 4;
    unsigned int fat_sector = fs->first_fat_sector + (fat_offset / fs->bs->bytes_per_sector);
    unsigned int ent_offset = fat_offset % fs->bs->bytes_per_sector;

    emmc_seek(fat_sector * 512);
    emmc_read(FAT_table, 512);

    unsigned int table_value = *(unsigned int *)&FAT_table[ent_offset];
    table_value &= 0x0fffffff;

    // end
    if (table_value >= 0x0FFFFFF8)
        return 0;
    return table_value;
}

unsigned int read_cluster(fatfs_t *fs, unsigned int cluster, unsigned char *buf)
{
    unsigned int cluster_sector = ((cluster - 2) * fs->bs->sectors_per_cluster) + fs->first_data_sector;
    unsigned int entries_bytes = fat_cluster_size(fs);

    emmc_seek(cluster_sector * 512);
    emmc_read(buf, entries_bytes);

    return entries_bytes;
}

unsigned int write_cluster(fatfs_t *fs, unsigned int cluster, unsigned char *buf)
{
    unsigned int cluster_sector = ((cluster - 2) * fs->bs->sectors_per_cluster) + fs->first_data_sector;
    unsigned int entries_bytes = fat_cluster_size(fs);

    emmc_seek(cluster_sector * 512);
    emmc_write(buf, entries_bytes);

    return entries_bytes;
}

void write_cluster_link(fatfs_t *fs, unsigned int cluster_no, unsigned int value)
{
    unsigned char FAT_table[512];
    unsigned int fat_offset = cluster_no * 4;
    unsigned int fat_sector = fs->first_fat_sector + (fat_offset / fs->bs->bytes_per_sector);
    unsigned int ent_offset = fat_offset % fs->bs->bytes_per_sector;

    emmc_seek(fat_sector * 512);
    emmc_read(FAT_table, 512);

    value &= 0x0FFFFFFF;
    *(unsigned int *)&FAT_table[ent_offset] = value;

    emmc_seek(fat_sector * 512);
    emmc_write(FAT_table, 512);
}

unsigned int link_free_cluster(fatfs_t *fs, unsigned int cluster)
{
    unsigned int free = free_cluster(fs);
    unsigned int last = cluster;
    while (true)
    {
        unsigned int next = next_cluster(fs, last);
        if (!next)
            break;
        last = next;
    }

    write_cluster_link(fs, last, free);
    write_cluster_link(fs, free, 0x0FFFFFFF);
    return free;
}

bool update_entry(unsigned int parent_cluster, fatfs_node_t *target_entry, bool clean_clusters)
{
    fatfs_t *fs = target_entry->fatfs;
    unsigned int cluster_sector = (parent_cluster - 2) * fs->bs->sectors_per_cluster + fs->first_data_sector;

    for (unsigned int sector = 0; sector < fs->bs->sectors_per_cluster; sector++)
    {
        unsigned char sector_buf[512];
        emmc_seek((cluster_sector + sector) * 512);
        emmc_read(sector_buf, 512);

        fat32_entry_t *entries = (fat32_entry_t *)sector_buf;
        for (unsigned int i = 0; i < 16; i++)
        {
            unsigned char x[12];
            memcpy(x, entries[i].name, 11);
            x[11] = '\0';

            unsigned char y[12];
            memcpy(y, target_entry->entry.name, 11);
            y[11] = '\0';

            if (ustrcmp(x, y) == 0)
            {
                memcpy(&entries[i], &target_entry->entry, sizeof(fat32_entry_t));

                unsigned int content = target_entry->cluster;
                unsigned int entries_bytes = fs->bs->sectors_per_cluster * fs->bs->bytes_per_sector;
                unsigned char clean[entries_bytes];
                memset(clean, 0, entries_bytes);

                while (content != 0 && clean_clusters)
                {
                    write_cluster(fs, content, clean);
                    content = next_cluster(fs, content);
                }

                emmc_seek((cluster_sector + sector) * 512);
                emmc_write(sector_buf, 512);
                return true;
            }
        }
    }

    unsigned int ncluster = next_cluster(fs, parent_cluster);
    if (ncluster != 0)
        return update_entry(ncluster, target_entry, clean_clusters);

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
    unsigned char *short_name = (unsigned char *)malloc(11);
    memset(short_name, ' ', 11);
    int dot = 0;

    for (unsigned long i = 0; i < strlen(name); i++)
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

        short_name[i] = utoupper(name[i]);
    }

    for (int i = dot + 1; i < dot + 4 && name[i]; i++)
        short_name[(i - dot - 1) + 8] = utoupper(name[i]);

    return short_name;
}

fatfs_node_t create_fatnode(fatfs_t *fs, unsigned int parent_cluster, const char *name, unsigned char attrs)
{
    unsigned char *short_name = generate_short(name);
    unsigned long namelen = strlen(name);
    unsigned char checksum = lfn_checksum(short_name);
    unsigned char *buf = malloc(fat_cluster_size(fs));

    while (true)
    {
        unsigned int entries_bytes = read_cluster(fs, parent_cluster, buf);
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

            lfn_entry_t lfn_entries[20];

            for (unsigned int j = 0; j < lfn_count; j++)
            {
                lfn_entry_t *lfn = (lfn_entry_t *)&buf[i + (lfn_count - j - 1) * 32];
                memset(lfn, 0, sizeof(lfn_entry_t));

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

                lfn_entries[j] = *lfn;
            }

            unsigned int offset = i + lfn_count * 32;
            fat32_entry_t *dir = (fat32_entry_t *)&buf[offset];

            unsigned int cluster = 0;

            memset(dir, 0, sizeof(struct fat32_dir_entry));
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

            unsigned int cluster_sector = ((parent_cluster - 2) * fs->bs->sectors_per_cluster) + fs->first_data_sector;

            emmc_seek(cluster_sector * 512);
            emmc_write(buf, entries_bytes);
            printf("Created file %s.\n", name);

            fatfs_node_t node;
            node.cluster = cluster;
            node.content_cluster = 0;
            node.parent_cluster = parent_cluster;
            node.entry = *dir;
            node.fatfs = fs;
            node.lfn_count = lfn_count;
            for (unsigned int i = 0; i < node.lfn_count; i++)
                node.lfn_entries[i] = lfn_entries[i];

            set_filename(fs, &node);

            free(short_name);
            free(buf);
            return node;
        }

        unsigned int next = next_cluster(fs, parent_cluster);
        if (next == 0)
            next = link_free_cluster(fs, parent_cluster);

        parent_cluster = next;
    }
}

unsigned long get_fatentries(fatfs_t *fs, unsigned int cluster, fatfs_node_t **bnodes)
{
    lfn_entry_t lfn_entries[20];
    unsigned int lfn_count = 0;

    fatfs_node_t *nodes = malloc(sizeof(fatfs_node_t));
    unsigned long nodes_capacity = 1;
    unsigned long nodes_count = 0;

    unsigned int clsize = fat_cluster_size(fs);
    unsigned char *buf = malloc(clsize);

    while (true)
    {
        unsigned int entries_bytes = read_cluster(fs, cluster, buf);

        for (unsigned int i = 0; i < entries_bytes; i += 32)
        {
            if (buf[i] == 0x0)
            {
                *bnodes = nodes;
                free(buf);
                return nodes_count;
            }

            if (buf[i] == 0xE5)
                continue;

            if (buf[i + 11] == 0x0F)
            {
                lfn_entry_t lfn = *((lfn_entry_t *)&buf[i]);
                lfn_entries[lfn_count] = lfn;
                lfn_count++;
                continue;
            }

            fat32_entry_t entry = *((fat32_entry_t *)&buf[i]);
            fatfs_node_t node;
            node.cluster = cluster;
            node.parent_cluster = cluster;
            node.content_cluster = (entry.cluster_high << 16) | entry.cluster_low;
            node.entry = entry;
            node.lfn_count = lfn_count;
            memcpy(node.lfn_entries, lfn_entries, node.lfn_count * sizeof(lfn_entry_t));

            node.fatfs = fs;
            set_filename(fs, &node);

            if (nodes_count == nodes_capacity)
            {
                nodes_capacity *= 2;

                fatfs_node_t *new_nodes = malloc(sizeof(fatfs_node_t) * nodes_capacity);
                memcpy(new_nodes, nodes, nodes_count * sizeof(fatfs_node_t));

                free(nodes);
                nodes = new_nodes;
            }

            nodes[nodes_count++] = node;

            lfn_count = 0;
            memset(lfn_entries, 0, sizeof(lfn_entries));
        }

        free(buf);
        cluster = next_cluster(fs, cluster);
        if (cluster == 0)
        {
            *bnodes = nodes;
            return nodes_count;
        }
    }
}

unsigned char *read_fatnode(fatfs_node_t node)
{
    unsigned int cluster = node.content_cluster;
    unsigned char *file = malloc(node.entry.size);
    unsigned int clsize = fat_cluster_size(node.fatfs);
    unsigned char *buf = malloc(clsize);
    unsigned int count = 0;

    while (true)
    {
        unsigned int bytes = read_cluster(node.fatfs, cluster, buf);

        for (unsigned int i = 0; i < bytes; i++)
            file[count + i] = buf[i];
        count += bytes;

        cluster = next_cluster(node.fatfs, cluster);
        if (cluster == 0)
            break;
    }

    free(buf);
    return file;
}

unsigned char *read_fatnode_at(fatfs_node_t node, unsigned long pos)
{
    unsigned int cluster = node.content_cluster;
    unsigned char *file = malloc(node.entry.size);
    unsigned int clsize = fat_cluster_size(node.fatfs);
    unsigned char *buf = malloc(clsize);
    unsigned int count = 0;

    for (unsigned long i = 0; i < pos; i++)
    {
        cluster = next_cluster(node.fatfs, cluster);
        if (cluster == 0)
            return 0;
    }

    unsigned int bytes = read_cluster(node.fatfs, cluster, buf);

    for (unsigned int i = 0; i < bytes; i++)
        file[count + i] = buf[i];
    count += bytes;

    free(buf);
    return file;
}

bool write_to_fatnode(fatfs_node_t *node, const char *cbuf, unsigned long size)
{
    unsigned char *buf = malloc(size);
    memcpy(buf, cbuf, size);

    unsigned int cluster = node->content_cluster;
    if (cluster == 0)
    {
        cluster = free_cluster(node->fatfs);
        write_cluster_link(node->fatfs, cluster, 0x0FFFFFFF);
        node->content_cluster = cluster;
        node->entry.cluster_low = cluster & 0xFFFF;
        node->entry.cluster_high = (cluster >> 16) & 0xFFFF;
    }

    unsigned long padded_size = ((size + 511) / 512) * 512;
    unsigned char *padded_buf = 0;
    if (size % 512 != 0)
    {
        padded_buf = malloc(padded_size);
        memset(padded_buf, 0, padded_size);
        memcpy(padded_buf, buf, size);
        buf = padded_buf;
    }

    for (unsigned long i = 0; i < padded_size; i += 512)
    {
        write_cluster(node->fatfs, cluster, buf + i);
        unsigned int next = next_cluster(node->fatfs, cluster);
        if (next == 0)
            next = link_free_cluster(node->fatfs, cluster);

        cluster = next;
    }

    node->entry.size = size;
    update_entry(node->parent_cluster, node, false);

    free(buf);
    if (padded_buf)
        free(padded_buf);

    return true;
}