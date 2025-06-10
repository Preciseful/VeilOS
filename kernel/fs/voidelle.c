#include <fs/voidelle.h>
#include <memory/memory.h>
#include <drivers/emmc.h>
#include <fs/mbr.h>
#include <lib/printf.h>
#include <limits.h>
#include <lib/string.h>

voidelle_t *voidelle_root(voidom_t *vdom)
{
    unsigned char buf[512];
    emmc_seek(vdom->partition_seek + vdom->voidlet->voidelle_size);
    emmc_read(buf, 512);

    voidelle_t *root = malloc(sizeof(voidelle_t));
    memcpy(root, buf, sizeof(voidelle_t));

    return root;
}

voidlet_t *get_voidlet(unsigned long offset)
{
    emmc_seek(offset);
    unsigned char buf[512];
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

    voidlet_t *vlet = malloc(sizeof(voidlet_t));
    memcpy(vlet, buf, sizeof(voidlet_t));
    if (memcmp("VOID", vlet->identifier, 4))
        return 0;

    return vlet;
}

voidom_t *voidelle_init()
{
    mbr_t mbr;
    emmc_seek(0);
    emmc_read((unsigned char *)&mbr, sizeof(mbr));

    if (mbr.bootSignature != BOOT_SIGNATURE)
    {
        printf("Bad MBR signature.\n");
        return 0;
    }

    voidlet_t *vlet = 0;
    unsigned long fs_start = 0;

    for (int i = 0; mbr.partitions[i].type != 0; i++)
    {
        vlet = get_voidlet((unsigned long)mbr.partitions[i].first_lba_sector * 512);
        if (vlet)
        {
            fs_start = (unsigned long)mbr.partitions[i].first_lba_sector * 512;
            printf("Found Voidelle on partition %d.\n", i + 1);
            break;
        }
    }

    if (!vlet)
        return 0;

    voidom_t *voidom = malloc(sizeof(voidom_t));
    voidom->partition_seek = fs_start;
    voidom->voidlet = vlet;
    voidom->root = voidelle_root(voidom);

    return voidom;
}

unsigned long ctz64(unsigned long x)
{
    if (x == 0)
        return 64;

    unsigned long n = 0;
    while ((x & 1) == 0)
    {
        x >>= 1;
        n++;
    }

    return n;
}

bool free_void(voidom_t *vdom, unsigned long block)
{
    // block will be this equation initially
    // (i * 512 * 8 + bit_index) * 512

    unsigned long i = block / (512 * 8);
    unsigned long bit_index = block % (512 * 8);

    unsigned char buf[512];
    emmc_seek(vdom->partition_seek + vdom->voidlet->voidmap + i * 512);
    emmc_read(buf, 512);

    buf[bit_index / 8] &= ~(1 << bit_index % 8);

    emmc_write(buf, 512);

    return true;
}

bool allocate_void(voidom_t *vdom, unsigned long *val)
{
    for (unsigned long i = 0; i < vdom->voidlet->voidmap_size; i++)
    {
        emmc_seek(vdom->partition_seek + vdom->voidlet->voidmap + i * 512);
        unsigned char buf[512];
        emmc_read(buf, 512);

        unsigned long *values = (unsigned long *)buf;
        int values_count = 512 / sizeof(unsigned long);
        int value_pos = -1;

        for (int j = 0; j < values_count; j++)
        {
            if (values[j] != UINT64_MAX)
            {
                value_pos = j;
                break;
            }
        }

        if (value_pos == -1)
            continue;

        unsigned long inverted = ~values[value_pos];
        unsigned long bit_pos = ctz64(inverted);
        unsigned long bit_index = value_pos * 64 + bit_pos;
        buf[bit_index / 8] |= (1 << (bit_index % 8));

        *val = (i * 512 * 8 + bit_index) * 512;
        emmc_write(buf, 512);

        return true;
    }

    return false;
}

voidite_t *create_voidite(voidom_t *vdom, char *write_buf, unsigned long write_buf_size)
{
    unsigned long free;
    allocate_void(vdom, &free);

    voidite_t *voidite = malloc(sizeof(voidite_t));
    voidite->next = 0;
    voidite->pos = free;

    memcpy(voidite->data, write_buf, write_buf_size < VOIDITE_CONTENT_SIZE ? write_buf_size : VOIDITE_CONTENT_SIZE);

    emmc_seek(vdom->partition_seek + free);
    emmc_write((unsigned char *)voidite, 512);

    return voidite;
}

voidelle_t *create_voidelle(voidom_t *vdom, unsigned long parent_pos, unsigned long flags, char *name)
{
    voidelle_t *parent = malloc(sizeof(voidelle_t));
    emmc_seek(vdom->partition_seek + parent_pos);
    emmc_read((unsigned char *)parent, 512);

    if (!(parent->flags & VOIDELLE_DIRECTORY))
        return 0;

    unsigned long voidelle_free;
    allocate_void(vdom, &voidelle_free);

    voidelle_t *voidelle = malloc(sizeof(voidelle_t));
    memcpy(voidelle->velle, "VELLE", 5);
    voidelle->pos = voidelle_free;
    voidelle->content = 0;
    voidelle->content_size = 0;
    for (int i = 0; i < 5; i++)
    {
        voidelle->create_date[i] = 0;
        voidelle->modify_date[i] = 0;
    }

    voidelle->create_year = 0;
    voidelle->modify_year = 0;
    voidelle->flags = flags;
    voidelle->owner_id = 0;
    for (int i = 0; i < 3; i++)
    {
        voidelle->owner_permission[i] = 0;
        voidelle->others_permission[i] = 0;
    }

    voidite_t *voidite = create_voidite(vdom, name, strlen(name));

    voidelle->name = voidite->pos;
    voidelle->next = 0;

    if (parent->content == 0)
    {
        parent->content = voidelle->pos;
        emmc_seek(vdom->partition_seek + parent->pos);
        emmc_write((unsigned char *)parent, 512);

        emmc_seek(vdom->partition_seek + voidelle->pos);
        emmc_write((unsigned char *)voidelle, 512);

        free(voidite);
        free(parent);
        return voidelle;
    }

    unsigned long neighbour_pos = parent->content;
    voidelle_t *neighbour = malloc(sizeof(voidelle_t));

    while (true)
    {
        emmc_seek(vdom->partition_seek + neighbour_pos);
        emmc_read((unsigned char *)neighbour, 512);

        if (neighbour->next == 0)
            break;

        neighbour_pos = neighbour->next;
    }

    neighbour->next = voidelle->pos;
    emmc_seek(vdom->partition_seek + neighbour_pos);
    emmc_write((unsigned char *)neighbour, 512);

    emmc_seek(vdom->partition_seek + voidelle->pos);
    emmc_write((unsigned char *)voidelle, 512);

    free(voidite);
    free(parent);
    free(neighbour);
    return voidelle;
}

unsigned char *voidelle_read_at(voidom_t *vdom, voidelle_t *velle, unsigned long pos)
{
    if (velle->content == 0)
        return 0;
    if (velle->flags & VOIDELLE_DIRECTORY)
        return 0;

    unsigned long voidite_pos = velle->content;
    voidite_t *last = malloc(sizeof(voidite_t));

    for (unsigned long i = 0; i < pos; i++)
    {
        if (i != 0)
            voidite_pos = last->next;

        unsigned char buf[512];
        emmc_seek(vdom->partition_seek + voidite_pos);
        emmc_read(buf, 512);
        memcpy(last, buf, 512);
    }

    return last->data;
}

void voidelle_write(voidom_t *vdom, voidelle_t *velle, char *write_buf, unsigned long write_buf_size)
{
    velle->content_size = write_buf_size;
    voidite_t *last = 0;

    while (write_buf_size)
    {
        voidite_t *voidite = create_voidite(vdom, write_buf, write_buf_size);
        if (last != 0)
            last->next = voidite->pos;
        else
            velle->content = voidite->pos;

        write_buf_size -= VOIDITE_CONTENT_SIZE;
        last = voidite;
    }
}

unsigned long get_voidelle_entries(voidom_t *vdom, unsigned long parent_pos, voidelle_t **bnodes)
{
    voidelle_t *parent = malloc(sizeof(voidelle_t));
    emmc_seek(vdom->partition_seek + parent_pos);
    emmc_read((unsigned char *)parent, 512);

    if (!(parent->flags & VOIDELLE_DIRECTORY))
        return 0;

    voidelle_t *nodes = malloc(sizeof(voidelle_t));
    unsigned long nodes_capacity = 1;
    unsigned long nodes_count = 0;

    unsigned long next = parent->content;
    unsigned char buf[512];

    while (next)
    {
        emmc_seek(vdom->partition_seek + next);
        emmc_read(buf, 512);
        voidelle_t entry = *((voidelle_t *)buf);

        if (nodes_count == nodes_capacity)
        {
            nodes_capacity *= 2;

            voidelle_t *new_nodes = malloc(sizeof(voidelle_t) * nodes_capacity);
            memcpy(new_nodes, nodes, nodes_count * sizeof(voidelle_t));

            free(nodes);
            nodes = new_nodes;
        }

        nodes[nodes_count++] = entry;
        next = entry.next;
    }

    *bnodes = nodes;
    free(parent);
    return nodes_count;
}

char *get_voidelle_name(voidom_t *vdom, unsigned long name_pos)
{
    unsigned char buf[512];
    char *file = malloc(VOIDITE_CONTENT_SIZE);
    unsigned long file_capacity = 1;
    unsigned long file_count = 0;
    voidite_t *vite = 0;

    while (name_pos)
    {
        if (file_count == file_capacity)
        {
            file_capacity *= 2;

            char *new_file = malloc(VOIDITE_CONTENT_SIZE * file_capacity);
            memcpy(new_file, vite, file_count * VOIDITE_CONTENT_SIZE);

            free(file);
            file = new_file;
        }

        emmc_seek(vdom->partition_seek + name_pos);
        emmc_read(buf, 512);
        vite = (voidite_t *)buf;

        memcpy(file + file_count * VOIDITE_CONTENT_SIZE, vite->data, VOIDITE_CONTENT_SIZE);
        file_count++;

        name_pos = vite->next;
    }

    return file;
}