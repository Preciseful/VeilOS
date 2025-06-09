#include <fs/voidelle.h>
#include <memory/memory.h>
#include <drivers/emmc.h>
#include <fs/mbr.h>
#include <lib/printf.h>
#include <limits.h>
#include <lib/string.h>

static unsigned long fs_start;

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

voidlet_t *voidelle_init()
{
    mbr_t mbr;
    emmc_seek(0);
    emmc_read((unsigned char *)&mbr, sizeof(mbr));

    if (mbr.bootSignature != BOOT_SIGNATURE)
    {
        printf("Bad MBR signature.\n");
        return 0;
    }

    for (int i = 0; mbr.partitions[i].type != 0; i++)
    {
        voidlet_t *vlet = get_voidlet((unsigned long)mbr.partitions[i].first_lba_sector * 512);
        if (vlet)
        {
            fs_start = (unsigned long)mbr.partitions[i].first_lba_sector * 512;
            printf("Found Voidelle on partition %d.\n", i + 1);
            return vlet;
        }
    }

    return 0;
}

voidelle_t *voidelle_root(voidlet_t *vlet)
{
    unsigned char buf[512];
    emmc_seek(fs_start + vlet->voidelle_size);
    emmc_read(buf, 512);

    voidelle_t *root = malloc(sizeof(voidelle_t));
    memcpy(root, buf, sizeof(voidelle_t));

    return root;
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

bool free_void(voidlet_t *vlet, unsigned long block)
{
    // block will be this equation initially
    // (i * 512 * 8 + bit_index) * 512

    unsigned long val = block / 512;
    unsigned long i = block / (512 * 8);
    unsigned long bit_index = block % (512 * 8);

    unsigned char buf[512];
    emmc_seek(fs_start + vlet->voidmap + i * 512);
    emmc_read(buf, 512);

    buf[bit_index / 8] &= ~(1 << bit_index % 8);

    emmc_write(buf, 512);

    return true;
}

bool allocate_void(voidlet_t *vlet, unsigned long *val)
{
    for (unsigned long i = 0; i < vlet->voidmap_size; i++)
    {
        emmc_seek(fs_start + vlet->voidmap + i * 512);
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

voidite_t *create_voidite(voidlet_t *vlet, char *write_buf, unsigned long write_buf_size)
{
    unsigned long free;
    allocate_void(vlet, &free);

    voidite_t *voidite = malloc(sizeof(voidite_t));
    voidite->next = 0;
    voidite->pos = free;

    memcpy(voidite->data, write_buf, write_buf_size < VOIDITE_CONTENT_SIZE ? write_buf_size : VOIDITE_CONTENT_SIZE);

    emmc_seek(fs_start + free);
    emmc_write((unsigned char *)voidite, 512);

    return voidite;
}

voidelle_t *create_voidelle(voidlet_t *vlet, unsigned long parent_pos, unsigned long flags, char *name)
{
    voidelle_t *parent = malloc(sizeof(voidelle_t));
    emmc_seek(fs_start + parent_pos);
    emmc_read((unsigned char *)parent, 512);

    if (!(parent->flags & VOIDELLE_DIRECTORY))
        return 0;

    unsigned long voidelle_free;
    allocate_void(vlet, &voidelle_free);

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

    voidite_t *voidite = create_voidite(vlet, name, strlen(name));

    voidelle->name = voidite->pos;
    voidelle->next = 0;

    if (parent->content == 0)
    {
        parent->content = voidelle->pos;
        emmc_seek(fs_start + parent->pos);
        emmc_write((unsigned char *)parent, 512);

        emmc_seek(fs_start + voidelle->pos);
        emmc_write((unsigned char *)voidelle, 512);

        free(voidite);
        free(parent);
        return voidelle;
    }

    unsigned long neighbour_pos = parent->content;
    voidelle_t *neighbour = malloc(sizeof(voidelle_t));

    while (true)
    {
        emmc_seek(fs_start + neighbour_pos);
        emmc_read((unsigned char *)neighbour, 512);

        if (neighbour->next == 0)
            break;

        neighbour_pos = neighbour->next;
    }

    neighbour->next = voidelle->pos;
    emmc_seek(fs_start + neighbour_pos);
    emmc_write((unsigned char *)neighbour, 512);

    emmc_seek(fs_start + voidelle->pos);
    emmc_write((unsigned char *)voidelle, 512);

    free(voidite);
    free(parent);
    free(neighbour);
    return voidelle;
}

unsigned char *voidelle_read_at(voidelle_t *velle, unsigned long pos)
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
        emmc_seek(fs_start + voidite_pos);
        emmc_read(buf, 512);
        memcpy(last, buf, 512);
    }

    return last->data;
}

void voidelle_write(voidlet_t *vlet, voidelle_t *velle, char *write_buf, unsigned long write_buf_size)
{
    velle->content_size = write_buf_size;
    voidite_t *last = 0;

    while (write_buf_size)
    {
        voidite_t *voidite = create_voidite(vlet, write_buf, write_buf_size);
        if (last != 0)
            last->next = voidite->pos;
        else
            velle->content = voidite->pos;

        write_buf_size -= VOIDITE_CONTENT_SIZE;
        last = voidite;
    }
}
