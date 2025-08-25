#include <fs/voidelle.h>
#include <limits.h>
#include <drivers/emmc.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <memory/memory.h>

Voidlet get_voidlet(Voidom *voidom)
{
    Voidlet voidlet;
    SeekInEMMC(voidom->partition.offset + 0);
    ReadFromEMMC((unsigned char *)&voidlet, sizeof(Voidlet));

    return voidlet;
}

Voidelle get_voidelle(Voidom *voidom, long seek)
{
    if (seek % VOID_SIZE != 0)
        LOG("get_voidelle: failed.\n");
    Voidelle voidelle;
    SeekInEMMC(voidom->partition.offset + seek);
    ReadFromEMMC((unsigned char *)&voidelle, sizeof(Voidelle));

    return voidelle;
}

Voidom *VoidelleInit(Partition partition)
{
    Voidom *voidom = malloc(sizeof(Voidom));
    voidom->partition = partition;
    voidom->voidlet = get_voidlet(voidom);
    voidom->root = get_voidelle(voidom, 512);

    return voidom;
}

char *get_voidelle_name(Voidom *voidom, Voidelle voidelle)
{
    char *velle_name = malloc(VOID_SIZE);
    velle_name[0] = '\0';
    unsigned long name_pos = voidelle.name;

    for (unsigned long i = 0; name_pos; i++)
    {
        if (i > 0)
        {
            char *old = velle_name;
            velle_name = malloc(VOID_SIZE * i);
            memcpy(velle_name, old, VOID_SIZE * (i - 1));
            free(old);
        }

        Voidite name;
        SeekInEMMC(voidom->partition.offset + name_pos);
        ReadFromEMMC((unsigned char *)&name, VOID_SIZE);
        name_pos = name.next;

        if (name_pos == 0)
            strcpy(velle_name + i * VOID_SIZE, name.data);
        else
            memcpy(velle_name + i * VOID_SIZE, name.data, VOIDITE_CONTENT_SIZE);
    }

    return velle_name;
}

unsigned long get_entries(Voidom *voidom, Voidelle voidelle, Voidelle **b_entries)
{
    if (voidelle.content == 0)
        return 0;

    Voidelle *entries = malloc(sizeof(Voidelle));
    unsigned long entries_capacity = 1;
    unsigned long entries_count = 0;

    unsigned long entry_pos = voidelle.content;
    for (unsigned long i = 0; entry_pos; i++)
    {
        if (entries_capacity == i)
        {
            Voidelle *old = entries;
            entries = malloc(sizeof(Voidelle) * entries_capacity * 2);
            memcpy(entries, old, sizeof(Voidelle) * entries_capacity);
            free(old);
            entries_capacity *= 2;
        }

        Voidelle entry = get_voidelle(voidom, entry_pos);
        entries[i] = entry;
        entry_pos = entry.next;
        entries_count++;
    }

    *b_entries = entries;
    return entries_count;
}

unsigned long get_free_section(Voidom *voidom)
{
    Voidlet voidlet = get_voidlet(voidom);

    for (unsigned long i = 0; i < voidlet.voidmap_size; i += VOID_SIZE)
    {
        unsigned char total_bits[VOID_SIZE];
        SeekInEMMC(voidom->partition.offset + voidlet.voidmap + i);
        ReadFromEMMC(total_bits, VOID_SIZE);
        unsigned long pos = 0;

        for (unsigned long j = 0; j < VOID_SIZE; j++)
        {
            unsigned char bits = total_bits[j];
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

            pos = ((i + j) * 8 + (7 - bit_pos)) * VOID_SIZE;
            total_bits[j] = bits;
            SeekInEMMC(voidom->partition.offset + voidlet.voidmap + i);
            WriteToEMMC(total_bits, VOID_SIZE);
        }

        if (pos != 0)
            return pos;
    }

    return 0;
}

void invalidate_section(Voidom *voidom, unsigned long pos)
{
    Voidlet voidlet = get_voidlet(voidom);

    unsigned long section = pos / VOID_SIZE;
    unsigned long i = section / 8;
    unsigned long sector_offset = (i / VOID_SIZE) * VOID_SIZE;
    int bit_pos = 7 - (section % 8);

    unsigned char buf[VOID_SIZE];
    SeekInEMMC(voidom->partition.offset + voidlet.voidmap + sector_offset);
    ReadFromEMMC(buf, VOID_SIZE);

    buf[i % VOID_SIZE] &= ~(1 << bit_pos);

    SeekInEMMC(voidom->partition.offset + voidlet.voidmap + sector_offset);
    WriteToEMMC(buf, VOID_SIZE);
}

bool create_voidelle(Voidom *voidom, const char *filename, unsigned long flags, Voidelle *b_voidelle)
{
    Voidelle voidelle;
    voidelle.pos = get_free_section(voidom);

    if (voidelle.pos == 0)
    {
        LOG("Failed to get free section.\n");
        return false;
    }

    unsigned long filename_len = strlen(filename) + 1;
    unsigned long last_pos = 0, init_pos = 0;
    for (unsigned long i = 0; i < filename_len; i += VOIDITE_CONTENT_SIZE)
    {
        unsigned long current_len = filename_len - i;
        unsigned long pos = get_free_section(voidom);

        if (init_pos == 0)
            init_pos = pos;
        if (last_pos != 0)
        {
            Voidite last_voidite;
            SeekInEMMC(voidom->partition.offset + last_pos);
            ReadFromEMMC((unsigned char *)&last_voidite, sizeof(Voidite));

            last_voidite.next = pos;

            SeekInEMMC(voidom->partition.offset + last_pos);
            WriteToEMMC((unsigned char *)&last_voidite, sizeof(Voidite));
        }

        Voidite voidite;
        voidite.next = 0;
        voidite.pos = pos;
        memcpy(voidite.data, filename + i, (current_len > VOIDITE_CONTENT_SIZE ? VOIDITE_CONTENT_SIZE : current_len));

        SeekInEMMC(voidom->partition.offset + voidite.pos);
        WriteToEMMC((unsigned char *)&voidite, sizeof(Voidite));

        last_pos = pos;
    }

    Voidite voidelle_name;
    SeekInEMMC(voidom->partition.offset + init_pos);
    ReadFromEMMC((unsigned char *)&voidelle_name, sizeof(Voidite));

    voidelle.name = voidelle_name.pos;
    voidelle.next = 0;
    voidelle.content = 0;
    voidelle.content_size = 0;
    voidelle.flags = flags;
    voidelle.owner_id = 0;
    voidelle.owner_permission = 0b111;
    voidelle.others_permission = 0;
    voidelle.create_year = 0;
    voidelle.modify_year = 0;
    memset(voidelle.create_date, 0, 5);
    memcpy(voidelle.modify_date, 0, 5);
    memcpy(voidelle.velle, "VELLE", 5);

    SeekInEMMC(voidom->partition.offset + voidelle.pos);
    WriteToEMMC((unsigned char *)&voidelle, sizeof(Voidelle));
    *b_voidelle = voidelle;

    return true;
}

bool GetVoidelleFromPath(Voidom *voidom, const char *path, Voidelle *b_voidelle)
{
    if (*path != VOIDELLE_ROOT_CHARACTER)
    {
        LOG("Paths must be absolute when using ls.\n");
        return false;
    }

    path++;

    Voidelle dir = get_voidelle(voidom, VOID_SIZE);
    for (unsigned long start = 0; start < strlen(path);)
    {
        unsigned long end = start;
        while (path[end] && path[end] != '/')
            end++;

        unsigned long folder_path_size = end - start;
        char *folder_name = malloc(folder_path_size + 1);
        folder_name[folder_path_size] = 0;
        memcpy(folder_name, path + start, folder_path_size);

        if (dir.content == 0)
        {
            LOG("Directory '%s' does not exist.\n", folder_name);
            free(folder_name);
            return false;
        }

        Voidelle *entries;
        unsigned long entries_count = get_entries(voidom, dir, &entries);
        bool found = false;

        for (unsigned long i = 0; i < entries_count; i++)
        {
            char *entry_name = get_voidelle_name(voidom, entries[i]);
            if (folder_path_size != strlen(entry_name))
            {
                free(entry_name);
                continue;
            }

            if (memcmp(folder_name, entry_name, folder_path_size) != 0)
            {
                free(entry_name);
                continue;
            }

            free(entry_name);
            dir = entries[i];
            found = true;
            break;
        }

        if (!found)
        {
            LOG("Directory '%s' does not exist.\n", folder_name);
            free(folder_name);
            if (entries_count > 0)
                free(entries);
            return false;
        }

        if (!(dir.flags & VOIDELLE_DIRECTORY) && path[end])
        {
            LOG("Cannot open files ('%s').\n", folder_name);
            free(folder_name);
            if (entries_count > 0)
                free(entries);
            return false;
        }

        start = end + 1;
        free(folder_name);
        if (entries_count > 0)
            free(entries);
    }

    *b_voidelle = dir;
    return true;
}

const char *get_parent_filename(Voidom *voidom, const char *path, Voidelle *parent, bool create_parents)
{
    if (path[0] != VOIDELLE_ROOT_CHARACTER)
    {
        LOG("Paths must be absolute. ('%s')\n", path);
        return "";
    }

    if (path[1] == '\0')
    {
        LOG("Cannot create a root directory. ('%s')\n", path);
        return "";
    }

    Voidelle dir;
    unsigned long slash = 0;
    unsigned long path_len = strlen(path);
    if (path_len == 1)
        return "";

    for (long i = path_len - 1; i >= 1; i--)
    {
        if (path[i] == '/')
        {
            slash = i;
            break;
        }
    }

    if (slash == 0)
        dir = get_voidelle(voidom, VOID_SIZE);
    else
    {
        unsigned long new_path_len = slash;

        char *new_path = malloc(new_path_len + 1);
        memcpy(new_path, path, new_path_len);
        new_path[new_path_len] = 0;

        if (create_parents)
        {
            for (unsigned long i = 0; i <= new_path_len; i++)
            {
                if (new_path[i] == '/' || new_path[i] == 0)
                {
                    new_path[i] = 0;
                    MakeVoidelle(voidom, new_path, VOIDELLE_DIRECTORY, false);
                    new_path[i] = '/';
                }
            }

            new_path[new_path_len] = 0;
        }

        if (!GetVoidelleFromPath(voidom, new_path, &dir))
        {
            LOG("Directory '%s' does not exist.\n", new_path);
            return "";
        }

        free(new_path);
    }

    if (parent != 0)
        *parent = dir;

    const char *filename = &(path[slash + 1]);
    return filename;
}

const char *GetVoidelleName(Voidom *voidom, const char *path)
{
    return get_parent_filename(voidom, path, 0, false);
}

bool MakeVoidelle(Voidom *voidom, const char *path, unsigned long flags, bool recursive)
{
    Voidelle dir;
    Voidelle voidelle;

    const char *filename = get_parent_filename(voidom, path, &dir, recursive);
    if (filename[0] == '\0')
        return false;

    if (!create_voidelle(voidom, filename, flags, &voidelle))
    {
        LOG("Failed to create a voidelle.\n");
        return false;
    }

    if (dir.content == 0)
    {
        dir.content = voidelle.pos;
        SeekInEMMC(voidom->partition.offset + dir.pos);
        WriteToEMMC((unsigned char *)&dir, sizeof(Voidelle));
    }
    else
    {
        Voidelle neighbour;
        SeekInEMMC(voidom->partition.offset + dir.content);
        ReadFromEMMC((unsigned char *)&neighbour, sizeof(Voidelle));

        char *neighbour_name = get_voidelle_name(voidom, neighbour);
        if (strcmp(neighbour_name, filename) == 0)
        {
            LOG("File %s already exists.\n", filename);
            free(neighbour_name);
            return false;
        }

        free(neighbour_name);

        while (neighbour.next)
        {
            SeekInEMMC(voidom->partition.offset + neighbour.next);
            ReadFromEMMC((unsigned char *)&neighbour, sizeof(Voidelle));

            neighbour_name = get_voidelle_name(voidom, neighbour);
            if (strcmp(neighbour_name, filename) == 0)
            {
                LOG("File %s already exists.\n", filename);
                free(neighbour_name);
                return false;
            }

            free(neighbour_name);
        }

        neighbour.next = voidelle.pos;
        SeekInEMMC(voidom->partition.offset + neighbour.pos);
        WriteToEMMC((unsigned char *)&neighbour, sizeof(Voidelle));
    }

    return true;
}

bool rm_file_from_voidelle(Voidom *voidom, Voidelle dir, const char *filename, bool ignore_content, Voidelle *bvoidelle)
{
    Voidelle child;
    Voidelle previous_child;
    unsigned long child_pos = dir.content;

    while (child_pos != 0)
    {
        SeekInEMMC(voidom->partition.offset + child_pos);
        ReadFromEMMC((unsigned char *)&child, sizeof(Voidelle));

        char *child_name = get_voidelle_name(voidom, child);
        if (strcmp(child_name, filename) == 0)
        {
            free(child_name);
            break;
        }

        free(child_name);

        previous_child = child;
        child_pos = child.next;
    }

    if (child_pos == 0)
    {
        LOG("File '%s' does not exist.\n", filename);
        return false;
    }

    if ((child.flags & VOIDELLE_DIRECTORY) && child.content != 0 && !ignore_content)
    {
        LOG("Directory '%s' is not empty.\n", filename);
        return false;
    }

    if (child_pos == dir.content)
    {
        dir.content = child.next;
        SeekInEMMC(voidom->partition.offset + dir.pos);
        WriteToEMMC((unsigned char *)&dir, sizeof(Voidelle));
    }
    else
    {
        previous_child.next = child.next;
        SeekInEMMC(voidom->partition.offset + previous_child.pos);
        WriteToEMMC((unsigned char *)&previous_child, sizeof(Voidelle));
    }

    unsigned long name_pos = child.name;
    Voidite name;
    while (name_pos)
    {
        SeekInEMMC(voidom->partition.offset + name_pos);
        ReadFromEMMC((unsigned char *)&name, sizeof(Voidite));

        invalidate_section(voidom, name_pos);
        name_pos = name.next;
    }

    unsigned long content_pos = child.content;
    Voidite content;
    while (content_pos && !(child.flags & VOIDELLE_DIRECTORY))
    {
        SeekInEMMC(voidom->partition.offset + content_pos);
        ReadFromEMMC((unsigned char *)&content, sizeof(Voidite));

        invalidate_section(voidom, content_pos);
        content_pos = content.next;
    }

    invalidate_section(voidom, child.pos);
    *bvoidelle = child;
    return true;
}

bool RemoveVoidelle(Voidom *voidom, const char *path, bool recursive)
{
    Voidelle dir;
    const char *filename = get_parent_filename(voidom, path, &dir, false);
    if (filename[0] == '\0')
        return false;

    Voidelle parent;
    if (!rm_file_from_voidelle(voidom, dir, filename, recursive, &parent))
        return false;

    if (recursive)
    {
        Voidelle child;
        unsigned long child_pos = parent.content;
        unsigned long path_len = strlen(path);

        while (child_pos != 0)
        {
            SeekInEMMC(voidom->partition.offset + child_pos);
            ReadFromEMMC((unsigned char *)&child, sizeof(Voidelle));

            char *child_name = get_voidelle_name(voidom, child);
            char *new_path = malloc(path_len + strlen(child_name) + 2);

            strcpy(new_path, path);
            new_path[path_len] = '/';
            new_path[path_len + 1] = 0;

            new_path = strcat(new_path, child_name);

            RemoveVoidelle(voidom, new_path, true);

            free(child_name);
            child_pos = child.next;
        }
    }

    return true;
}

bool WriteToVoidelle(Voidom *voidom, Voidelle *voidelle, char *data, unsigned long size)
{
    if (voidelle->flags & VOIDELLE_DIRECTORY)
    {
        LOG("Cannot write to directory.\n");
        return false;
    }

    unsigned long old_content = voidelle->content;
    while (old_content)
    {
        Voidite voidite;
        SeekInEMMC(voidom->partition.offset + old_content);
        ReadFromEMMC((unsigned char *)&voidite, sizeof(Voidite));

        invalidate_section(voidom, voidite.pos);
        old_content = voidite.next;
    }

    unsigned long data_len = size;
    unsigned long last_pos = 0, init_pos = 0;

    for (unsigned long i = 0; i < data_len; i += VOIDITE_CONTENT_SIZE)
    {
        unsigned long current_len = data_len - i;
        unsigned long pos = get_free_section(voidom);

        if (init_pos == 0)
            init_pos = pos;
        if (last_pos != 0)
        {
            Voidite last_voidite;
            SeekInEMMC(voidom->partition.offset + last_pos);
            ReadFromEMMC((unsigned char *)&last_voidite, sizeof(Voidite));

            last_voidite.next = pos;

            SeekInEMMC(voidom->partition.offset + last_pos);
            WriteToEMMC((unsigned char *)&last_voidite, sizeof(Voidite));
        }

        Voidite voidite;
        voidite.next = 0;
        voidite.pos = pos;
        memcpy(voidite.data, data + i, (current_len > VOIDITE_CONTENT_SIZE ? VOIDITE_CONTENT_SIZE : current_len));

        SeekInEMMC(voidom->partition.offset + voidite.pos);
        WriteToEMMC((unsigned char *)&voidite, sizeof(Voidite));

        last_pos = pos;
    }

    voidelle->content_size = data_len;
    voidelle->content = init_pos;
    SeekInEMMC(voidom->partition.offset + voidelle->pos);
    WriteToEMMC((unsigned char *)&voidelle, sizeof(Voidelle));

    return true;
}

bool ReadVoidelleAt(Voidom *voidom, Voidelle voidelle, unsigned long index, Voidite *buf)
{
    if (voidelle.flags & VOIDELLE_DIRECTORY)
    {
        LOG("Cannot read contents of directories.\n");
        return false;
    }

    uint64_t data_pos = voidelle.content;

    for (unsigned long i = 0; data_pos; i++)
    {
        Voidite data;
        SeekInEMMC(voidom->partition.offset + data_pos);
        ReadFromEMMC((unsigned char *)&data, sizeof(Voidite));

        if (i == index)
        {
            memcpy(buf, &data, sizeof(Voidite));
            return true;
        }

        data_pos = data.next;
    }

    return false;
}