#include <fs/voidelle.h>
#include <drivers/emmc.h>
#include <lib/printf.h>
#include <lib/string.h>

Voidlet get_voidlet(Voidom voidom)
{
    Voidlet voidlet;
    SeekInEMMC(voidom.partition.offset + 0);
    ReadFromEMMC((unsigned char *)&voidlet, VOID_SIZE);

    return voidlet;
}

Voidelle get_root(Voidom voidom)
{
    Voidelle root;
    SeekInEMMC(voidom.partition.offset + VOID_SIZE);
    ReadFromEMMC((unsigned char *)&root, VOID_SIZE);

    return root;
}

unsigned long get_free_section(Voidom voidom)
{
    for (unsigned long i = 0; i < voidom.voidlet.voidmap_size; i++)
    {
        unsigned char total_bits[512];
        unsigned char bits;
        SeekInEMMC(voidom.partition.offset + voidom.voidlet.voidmap + (i / 512));
        ReadFromEMMC(total_bits, VOID_SIZE);
        bits = total_bits[i % 512];

        int bit_pos = 0;

        if (bits == UINT8_MAX)
            continue;

        if (bits == 0)
        {
            bits = 0b10000000;
            bit_pos = 7;
        }
        else
        {
            bit_pos = __builtin_ctz(bits);
            bit_pos--;
            bits |= (1 << bit_pos);
        }

        uint64_t pos = (i * 8 + (7 - bit_pos)) * VOID_SIZE;
        total_bits[i % 512] = bits;

        SeekInEMMC(voidom.partition.offset + voidom.voidlet.voidmap + (i / 512));
        WriteToEMMC(total_bits, VOID_SIZE);

        LOG("returned pos: %lu\n", pos);
        return pos;
    }

    return 0;
}

void invalidate_section(Voidom voidom, unsigned long pos)
{
    unsigned long section = pos / VOID_SIZE;
    unsigned long i = section / 8;
    unsigned long sector_offset = (i / VOID_SIZE) * VOID_SIZE;
    int bit_pos = 7 - (section % 8);

    unsigned char buf[VOID_SIZE];
    SeekInEMMC(voidom.partition.offset + voidom.voidlet.voidmap + sector_offset);
    ReadFromEMMC(buf, VOID_SIZE);

    buf[i % VOID_SIZE] &= ~(1 << bit_pos);

    SeekInEMMC(voidom.partition.offset + voidom.voidlet.voidmap + sector_offset);
    WriteToEMMC(buf, VOID_SIZE);
}

void get_voidelle_name(Voidom voidom, Voidelle voidelle, char *buffer)
{
    unsigned long pos = voidelle.name;
    unsigned long bytes_left = voidelle.name_size;

    while (pos)
    {
        Voidite voidite;
        ReadVoid(voidom, &voidite, pos);

        unsigned long read_count = bytes_left < VOIDITE_CONTENT_SIZE ? bytes_left : VOIDITE_CONTENT_SIZE;
        memcpy(buffer + voidelle.name_size - bytes_left, voidite.data, read_count);

        bytes_left -= read_count;
        pos = voidite.next;
    }
}

bool get_prev_neighbour(Voidom voidom, Voidelle parent, Voidelle voidelle, Voidelle *prev)
{
    if (parent.content == 0)
        return false;

    unsigned long pos = parent.content;
    while (pos)
    {
        Voidelle neighbour;
        ReadVoid(voidom, &neighbour, pos);

        if (neighbour.next == voidelle.pos)
        {
            *prev = neighbour;
            return true;
        }

        pos = neighbour.next;
    }

    return false;
}

void clear_voidites(Voidom voidom, unsigned long pos)
{
    while (pos)
    {
        Voidite voidite;
        ReadVoid(voidom, &voidite, pos);
        invalidate_section(voidom, voidite.pos);

        pos = voidite.next;
    }
}

unsigned long populate_data(Voidom voidom, Voidite *voidite, const void *data, unsigned long size)
{
    unsigned long initial_size = size;
    unsigned long void_count = ((size + VOIDITE_CONTENT_SIZE - 1) / VOIDITE_CONTENT_SIZE) + 1;
    Voidite *sections = malloc(sizeof(Voidite) * void_count);

    for (unsigned long i = 0; i < void_count; i++)
    {
        unsigned long pos = get_free_section(voidom);

        Voidite voidite;
        voidite.pos = pos;
        voidite.next = 0;

        unsigned long bytes = size < VOIDITE_CONTENT_SIZE ? size : VOIDITE_CONTENT_SIZE;
        memcpy(voidite.data, data, bytes);

        data += bytes;
        size -= bytes;

        sections[i] = voidite;

        if (size == 0)
        {
            void_count = i + 1;
            break;
        }
    }

    for (unsigned long i = 0; i < void_count; i++)
    {
        if (i < void_count - 1)
            sections[i].next = sections[i + 1].pos;

        UpdateVoidProperties(voidom, &sections[i], sections[i].pos);
    }

    *voidite = sections[0];
    free(sections);
    return initial_size - size;
}

bool VoidelleInit(Voidom *voidom, Partition partition)
{
    voidom->partition = partition;
    voidom->root = get_root(*voidom);
    voidom->voidlet = get_voidlet(*voidom);

    if (memcmp(voidom->voidlet.identifier, "VOID", 4) != 0)
        LOG("Identifier is not VOID.\n");
    LOG("Voidelle is found on: %s at %lu.\n", voidom->partition.name, voidom->partition.offset);
    LOG("%c %c %c %c\n", voidom->voidlet.identifier[0], voidom->voidlet.identifier[1], voidom->voidlet.identifier[2], voidom->voidlet.identifier[3]);
    LOG("Voidmap is at %lu.\n", voidom->voidlet.voidmap);

    return true;
}

void ReadVoid(Voidom voidom, void *void_section, unsigned long pos)
{
    SeekInEMMC(voidom.partition.offset + pos);
    ReadFromEMMC(void_section, VOID_SIZE);
}

void UpdateVoidProperties(Voidom voidom, void *void_section, unsigned long pos)
{
    SeekInEMMC(voidom.partition.offset + pos);
    WriteToEMMC(void_section, VOID_SIZE);
}

Voidelle CreateVoidelle(Voidom voidom, Voidelle *parent, const char *name, unsigned long flags)
{
    Voidelle voidelle;
    Voidite name_voidite;
    unsigned long name_size = strlen(name) + 1;
    populate_data(voidom, &name_voidite, name, name_size);

    strcpy(voidelle.velle, "VELLE");
    voidelle.content = 0;
    voidelle.content_size = 0;
    voidelle.create_year = 0;
    voidelle.modify_year = 0;
    memset(voidelle.create_date, 0, 5);
    memset(voidelle.modify_date, 0, 5);
    voidelle.flags = flags;
    voidelle.next = 0;
    voidelle.others_permission = 0;
    voidelle.owner_permission = 0;
    voidelle.owner_id = 0;
    voidelle.pos = 0;
    voidelle.name = name_voidite.pos;
    voidelle.name_size = name_size;

    Voidelle prev;
    bool found_previous = get_prev_neighbour(voidom, *parent, voidelle, &prev);
    // we maintain voidelle.pos 0 until this point in order for
    // get_prev_neighbour to bring the last voidelle within parent
    voidelle.pos = get_free_section(voidom);

    UpdateVoidProperties(voidom, &voidelle, voidelle.pos);

    if (found_previous)
    {
        prev.next = voidelle.pos;
        UpdateVoidProperties(voidom, &prev, prev.pos);
    }
    else
    {
        parent->content = voidelle.pos;
        UpdateVoidProperties(voidom, parent, parent->pos);
    }

    return voidelle;
}

unsigned long WriteToVoidelle(Voidom voidom, Voidelle *voidelle, const void *data, unsigned long size)
{
    if (voidelle->flags & VOIDELLE_DIRECTORY)
        return 0;

    clear_voidites(voidom, voidelle->content);

    Voidite content;
    unsigned long data_count = populate_data(voidom, &content, data, size);
    voidelle->content = content.pos;
    voidelle->content_size = size;

    UpdateVoidProperties(voidom, voidelle, voidelle->pos);

    return data_count;
}

void RemoveVoidelle(Voidom voidom, Voidelle *parent, Voidelle *voidelle)
{
    Voidelle prev;
    if (get_prev_neighbour(voidom, *parent, *voidelle, &prev))
    {
        prev.next = voidelle->next;
        UpdateVoidProperties(voidom, &prev, prev.pos);
    }
    else
    {
        parent->content = voidelle->next;
        UpdateVoidProperties(voidom, parent, parent->pos);
    }

    clear_voidites(voidom, voidelle->name);
    clear_voidites(voidom, voidelle->content);
    invalidate_section(voidom, voidelle->pos);
}