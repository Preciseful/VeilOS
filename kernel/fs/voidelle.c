#include <lib/string.h>
#include <drivers/emmc.h>
#include <stdint.h>
#include <stddef.h>

#include "voidelle.h"

// ALL disk_* functions should be swapped for your disk handling

// seek is relative to the partition's start
void disk_seek(Voidom voidom, uint64_t seek)
{
    SeekInEMMC(voidom.disk.offset + seek);
}

void disk_read(Voidom voidom, void *buf, uint64_t size)
{
    unsigned char temp[VOID_SIZE];
    ReadFromEMMC(temp, VOID_SIZE);

    memcpy(buf, temp, size);
}

void disk_write(Voidom voidom, void *buf, uint64_t size)
{
    unsigned char temp[VOID_SIZE];
    memset(temp, 0, VOID_SIZE);
    memcpy(temp, buf, size);

    WriteToEMMC(temp, VOID_SIZE);
}

long disk_size(Voidom voidom)
{
    return voidom.disk.backing_data.num_sectors * 512;
}

uint64_t get_free_void(Voidom voidom)
{
    uint8_t total_bits[VOID_SIZE];

    for (uint64_t i = 0; i < voidom.voidlet.voidmap_size; i++)
    {
        if (i % VOID_SIZE == 0)
        {
            disk_seek(voidom, voidom.voidlet.voidmap + (i / VOID_SIZE));
            disk_read(voidom, total_bits, VOID_SIZE);
        }

        uint8_t bits = total_bits[i % VOID_SIZE];

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
        total_bits[i % VOID_SIZE] = bits;

        disk_seek(voidom, voidom.voidlet.voidmap + (i / VOID_SIZE));
        disk_write(voidom, total_bits, VOID_SIZE);

        return pos;
    }

    return 0;
}

void invalidate_section(Voidom voidom, uint64_t pos)
{
    uint64_t bit_index = pos;
    uint64_t byte_index = bit_index / 8;
    int bit_pos = 7 - (bit_index % 8);

    uint8_t buf;
    disk_seek(voidom, voidom.voidlet.voidmap + byte_index);
    disk_read(voidom, &buf, 1);

    buf &= ~(1 << bit_pos);

    disk_seek(voidom, voidom.voidlet.voidmap + byte_index);
    disk_write(voidom, &buf, 1);
}

bool write_void(Voidom voidom, void *buf, uint64_t position, uint64_t size)
{
    if (size > VOID_SIZE)
        return false;

    uint8_t cpy_buf[VOID_SIZE];
    memset(cpy_buf, 0, VOID_SIZE);
    memcpy(cpy_buf, buf, size);

    disk_seek(voidom, position);
    disk_write(voidom, cpy_buf, VOID_SIZE);

    return true;
}

bool read_void(Voidom voidom, void *buf, uint64_t position, uint64_t size)
{
    if (size > VOID_SIZE)
        return false;

    disk_seek(voidom, position);
    disk_read(voidom, buf, size);
    return true;
}

void clear_voidites_after(Voidom voidom, Voidite *start)
{
    uint64_t pos = start->next_voidite;

    while (pos)
    {
        Voidite voidite;
        read_void(voidom, &voidite, pos, sizeof(Voidite));
        pos = voidite.next_voidite;

        voidite.next_voidite = 0;
        write_void(voidom, &voidite, voidite.position, sizeof(Voidite));
        invalidate_section(voidom, voidite.position);
    }

    start->next_voidite = 0;
    write_void(voidom, start, start->position, sizeof(Voidite));
}

void clear_voidelle_content(Voidom voidom, Voidelle *voidelle)
{
    if (voidelle->content_voidelle == 0)
        return;

    if (voidelle->flags & VOIDELLE_DIRECTORY)
    {
        uint64_t pos = voidelle->content_voidelle;

        while (pos)
        {
            Voidelle child;
            read_void(voidom, &child, pos, sizeof(Voidelle));

            remove_voidelle(voidom, voidelle, child, true);
            clear_voidelle_content(voidom, &child);
            clear_voidelle_name(voidom, &child);

            pos = child.next_voidelle;
        }
    }
    else
    {
        Voidite voidite;
        read_void(voidom, &voidite, voidelle->content_voidelle, sizeof(Voidite));

        clear_voidites_after(voidom, &voidite);
        invalidate_section(voidom, voidite.position);
    }

    voidelle->content_voidelle = 0;
    write_void(voidom, voidelle, voidelle->position, sizeof(Voidelle));
}

void clear_voidelle_name(Voidom voidom, Voidelle *voidelle)
{
    if (voidelle->name_voidelle == 0)
        return;

    Voidite voidite;
    read_void(voidom, &voidite, voidelle->name_voidelle, sizeof(Voidite));

    clear_voidites_after(voidom, &voidite);
    invalidate_section(voidom, voidite.position);

    voidelle->name_voidelle = 0;
    write_void(voidom, voidelle, voidelle->position, sizeof(Voidelle));
}

void fill_content_voidites(Voidom voidom, Voidelle *voidelle, unsigned long count)
{
    if (!voidelle->content_voidelle)
    {
        Voidite content;
        content.position = get_free_void(voidom);
        content.next_voidite = 0;
        memset(content.data, 0, VOIDITE_CONTENT_SIZE);

        voidelle->content_voidelle = content.position;

        write_void(voidom, voidelle, voidelle->position, sizeof(Voidelle));
        write_void(voidom, &content, content.position, sizeof(Voidite));
    }

    Voidite last_voidite;
    uint64_t pos = voidelle->content_voidelle;

    unsigned long existing = 0;

    while (pos)
    {
        read_void(voidom, &last_voidite, pos, sizeof(Voidite));
        pos = last_voidite.next_voidite;
        existing++;
    }

    if (count <= existing)
        return;
    count -= existing;

    for (size_t i = 0; i < count; i++)
    {
        Voidite next_voidite;
        next_voidite.next_voidite = 0;
        next_voidite.position = get_free_void(voidom);
        memset(next_voidite.data, 0, VOIDITE_CONTENT_SIZE);

        last_voidite.next_voidite = next_voidite.position;

        write_void(voidom, &last_voidite, last_voidite.position, sizeof(Voidite));
        write_void(voidom, &next_voidite, next_voidite.position, sizeof(Voidite));

        last_voidite = next_voidite;
    }
}

void fill_name_voidites(Voidom voidom, Voidelle *voidelle, unsigned long count)
{
    if (!voidelle->name_voidelle)
    {
        Voidite name;
        name.position = get_free_void(voidom);
        name.next_voidite = 0;
        memset(name.data, 0, VOIDITE_CONTENT_SIZE);

        voidelle->name_voidelle = name.position;

        write_void(voidom, voidelle, voidelle->position, sizeof(Voidelle));
        write_void(voidom, &name, name.position, sizeof(Voidite));
    }

    Voidite last_voidite;
    uint64_t pos = voidelle->name_voidelle;

    while (pos)
    {
        read_void(voidom, &last_voidite, pos, sizeof(Voidite));
        pos = last_voidite.next_voidite;
        count--;
    }

    for (size_t i = 0; i < count; i++)
    {
        Voidite next_voidite;
        next_voidite.next_voidite = 0;
        next_voidite.position = get_free_void(voidom);
        memset(next_voidite.data, 0, VOIDITE_CONTENT_SIZE);

        last_voidite.next_voidite = next_voidite.position;

        write_void(voidom, &last_voidite, last_voidite.position, sizeof(Voidite));
        write_void(voidom, &next_voidite, next_voidite.position, sizeof(Voidite));

        last_voidite = next_voidite;
    }
}

uint64_t populate_voidite_data(Voidom voidom, Voidite *first_voidite_buf, const void *data, uint64_t size)
{
    uint64_t initial_size = size;
    uint64_t void_count = ((size + VOIDITE_CONTENT_SIZE - 1) / VOIDITE_CONTENT_SIZE);
    Voidite *sections = malloc(sizeof(Voidite) * void_count);

    for (uint64_t i = 0; i < void_count; i++)
    {
        uint64_t pos = get_free_void(voidom);

        Voidite voidite;
        memset(&voidite, 0, sizeof(Voidite));
        voidite.position = pos;
        voidite.next_voidite = 0;

        uint64_t bytes = size < VOIDITE_CONTENT_SIZE ? size : VOIDITE_CONTENT_SIZE;
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

    for (uint64_t i = 0; i < void_count; i++)
    {
        if (i < void_count - 1)
            sections[i].next_voidite = sections[i + 1].position;

        write_void(voidom, &sections[i], sections[i].position, sizeof(Voidite));
    }

    *first_voidite_buf = sections[0];
    free(sections);
    return initial_size - size;
}

verror_t create_voidlet(Voidom *voidom)
{
    long file_size = disk_size(*voidom);
    long voidmap_size = (file_size / VOID_SIZE + 7) / 8;
    long bitmap_offset = file_size - voidmap_size;
    long voidmap = bitmap_offset - bitmap_offset % VOID_SIZE;

    Voidlet voidlet;
    memcpy(voidlet.header, "VOID", 4);
    voidlet.void_size = VOID_SIZE;
    voidlet.voidmap_size = voidmap_size;
    voidlet.voidmap = voidmap;

    if (!write_void(*voidom, &voidlet, 0, sizeof(Voidlet)))
        return UNKNOWN_ERROR;

    voidom->voidlet = voidlet;

    uint8_t zero_buf[VOID_SIZE];
    memset(zero_buf, 0, VOID_SIZE);
    uint8_t first_bits = 0b10000000;

    for (uint64_t i = 0; i < voidmap_size; i += VOID_SIZE)
        write_void(*voidom, zero_buf, voidmap + i, VOID_SIZE);

    write_void(*voidom, &first_bits, voidmap, sizeof(uint8_t));

    return SUCCESS;
}

verror_t create_voidelle(Voidom voidom, Voidelle *buf, const char *name, enum Voidelle_Flags flags, uint64_t owner_id, uint8_t owner_perm, uint8_t other_perm)
{
    uint64_t voidelle_position = get_free_void(voidom);

    Voidelle voidelle;
    memcpy(voidelle.header, "VELLE", 5);
    voidelle.flags = flags;
    voidelle.name_voidelle_size = strlen(name) + 1;
    voidelle.content_voidelle = 0;
    voidelle.content_voidelle_size = 0;
    voidelle.next_voidelle = 0;
    voidelle.position = voidelle_position;
    voidelle.creation_seconds = 0;
    voidelle.modification_seconds = 0;
    voidelle.access_seconds = 0;
    voidelle.owner_id = owner_id;
    voidelle.other_permission = other_perm;
    voidelle.owner_permission = owner_perm;

    Voidite name_voidite;
    populate_voidite_data(voidom, &name_voidite, name, voidelle.name_voidelle_size);

    voidelle.name_voidelle = name_voidite.position;

    if (!write_void(voidom, &voidelle, voidelle.position, sizeof(Voidelle)))
        return UNKNOWN_ERROR;

    memcpy(buf, &voidelle, sizeof(voidelle));
    return SUCCESS;
}

verror_t get_voidelle_name(Voidom voidom, Voidelle voidelle, char *buf)
{
    uint64_t read_bytes = 0;
    uint64_t size = voidelle.name_voidelle_size;
    uint64_t pos = voidelle.name_voidelle;

    while (pos)
    {
        uint64_t aligned_size = size > VOIDITE_CONTENT_SIZE ? VOIDITE_CONTENT_SIZE : size;

        Voidite voidite;
        read_void(voidom, &voidite, pos, sizeof(Voidite));
        memcpy(buf + read_bytes, voidite.data, aligned_size);

        size -= aligned_size;
        read_bytes += aligned_size;

        pos = voidite.next_voidite;
    }

    return SUCCESS;
}

bool get_content_voidite_at(Voidom voidom, Voidelle voidelle, Voidite *buf, unsigned long index)
{
    uint64_t content_pos = voidelle.content_voidelle;

    for (unsigned long i = 0; i < index; i++)
    {
        if (content_pos == 0)
            return false;

        Voidite current_voidite;
        read_void(voidom, &current_voidite, content_pos, sizeof(Voidite));

        content_pos = current_voidite.next_voidite;
    }

    if (content_pos == 0)
        return false;

    read_void(voidom, buf, content_pos, sizeof(Voidite));
    return true;
}

bool get_name_voidite_at(Voidom voidom, Voidelle voidelle, Voidite *buf, unsigned long index)
{
    uint64_t name_pos = voidelle.name_voidelle;

    for (unsigned long i = 0; i < index; i++)
    {
        if (name_pos == 0)
            return false;

        Voidite current_voidite;
        read_void(voidom, &current_voidite, name_pos, sizeof(Voidite));

        name_pos = current_voidite.next_voidite;
    }

    if (name_pos == 0)
        return false;

    read_void(voidom, buf, name_pos, sizeof(Voidite));
    return true;
}

unsigned long read_voidelle(Voidom voidom, Voidelle voidelle, unsigned long seek, void *buf, unsigned long size)
{
    if (voidelle.flags & VOIDELLE_DIRECTORY)
        return FILE_IS_DIRECTORY;

    uint8_t *current_buf = buf;

    uint64_t first_voidite = seek / VOIDITE_CONTENT_SIZE;
    uint64_t last_voidite = (seek + size - 1) / VOIDITE_CONTENT_SIZE;
    uint64_t start = seek % VOIDITE_CONTENT_SIZE;
    uint64_t end = (seek + size) % VOIDITE_CONTENT_SIZE;

    uint64_t bytes_left = size;

    for (unsigned long voidite_index = first_voidite; voidite_index <= last_voidite; voidite_index++)
    {
        Voidite voidite;

        if (!get_content_voidite_at(voidom, voidelle, &voidite, voidite_index))
            return current_buf - (uint8_t *)buf;

        uint8_t *cpy_start = voidite.data;
        uint64_t cpy_len = VOIDITE_CONTENT_SIZE;

        if (voidite_index == first_voidite)
        {
            cpy_start += start;
            cpy_len -= start;
        }

        if (voidite_index == last_voidite)
            cpy_len = (end == 0) ? size : end;

        if (cpy_len > bytes_left)
            cpy_len = bytes_left;

        memcpy(current_buf, cpy_start, cpy_len);

        current_buf += cpy_len;
        bytes_left -= cpy_len;
    }

    return current_buf - (uint8_t *)buf;
}

uint64_t get_previous_link_position(Voidom voidom, Voidelle parent, Voidelle voidelle, Voidelle *buf)
{
    Voidelle neighbour;
    uint64_t pos = parent.content_voidelle;

    if (pos == voidelle.position)
        return pos;

    while (pos)
    {
        read_void(voidom, &neighbour, pos, sizeof(neighbour));

        if (neighbour.next_voidelle == voidelle.position)
            break;

        pos = neighbour.next_voidelle;
    }

    if (neighbour.next_voidelle != voidelle.position)
        return 0;

    return neighbour.position;
}

void swap_voidelles(Voidom voidom, Voidelle *first, Voidelle *second, int swaps)
{
    uint64_t t;
    t = first->position;
    first->position = second->position;
    second->position = t;

    t = first->next_voidelle;
    first->next_voidelle = second->next_voidelle;
    second->next_voidelle = t;

    // switch name
    if (swaps & 0x1)
    {
        t = first->name_voidelle;
        first->name_voidelle = second->name_voidelle;
        second->name_voidelle = t;

        t = first->name_voidelle_size;
        first->name_voidelle_size = second->name_voidelle_size;
        second->name_voidelle_size = t;
    }

    // switch content
    if (swaps & 0x2)
    {
        t = first->content_voidelle;
        first->content_voidelle = second->content_voidelle;
        second->content_voidelle = t;

        t = first->content_voidelle_size;
        first->content_voidelle_size = second->content_voidelle_size;
        second->content_voidelle_size = t;
    }

    write_void(voidom, first, first->position, sizeof(Voidelle));
    write_void(voidom, second, second->position, sizeof(Voidelle));
}

bool remove_voidelle(Voidom voidom, Voidelle *parent, Voidelle voidelle, bool invalidate)
{
    if (parent->content_voidelle == voidelle.position)
    {
        parent->content_voidelle = voidelle.next_voidelle;

        write_void(voidom, parent, parent->position, sizeof(Voidelle));

        if (invalidate)
            invalidate_section(voidom, voidelle.position);

        return true;
    }
    else
    {
        Voidelle neighbour;
        uint64_t pos = parent->content_voidelle;

        while (pos)
        {
            read_void(voidom, &neighbour, pos, sizeof(neighbour));

            if (neighbour.next_voidelle == voidelle.position)
                break;

            pos = neighbour.next_voidelle;
        }

        if (neighbour.next_voidelle != voidelle.position)
            return false;

        neighbour.next_voidelle = voidelle.next_voidelle;
        write_void(voidom, &neighbour, neighbour.position, sizeof(neighbour));

        if (invalidate)
            invalidate_section(voidom, voidelle.position);

        return true;
    }

    return false;
}

void add_voidelle(Voidom voidom, Voidelle *parent, Voidelle *voidelle)
{
    voidelle->next_voidelle = parent->content_voidelle;
    parent->content_voidelle = voidelle->position;

    write_void(voidom, voidelle, voidelle->position, sizeof(Voidelle));
    write_void(voidom, parent, parent->position, sizeof(Voidelle));
}

void add_voidelle_with_check(Voidom voidom, Voidelle *parent, Voidelle voidelle)
{
    if (!parent->content_voidelle)
    {
        parent->content_voidelle = voidelle.position;
        write_void(voidom, parent, parent->position, sizeof(Voidelle));
    }
    else
    {
        Voidelle neighbour;
        unsigned long pos = parent->content_voidelle;

        while (pos)
        {
            if (pos == voidelle.position)
                return;

            read_void(voidom, &neighbour, pos, sizeof(Voidelle));
            pos = neighbour.next_voidelle;
        }

        neighbour.next_voidelle = voidelle.position;
        write_void(voidom, &neighbour, neighbour.position, sizeof(Voidelle));
    }
}

void VoidelleFSInit(Voidom *voidom, Partition partition)
{
    voidom->disk = partition;
    read_void(*voidom, &voidom->root, VOID_SIZE, sizeof(Voidelle));
}