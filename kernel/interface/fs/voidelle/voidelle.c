#include <fs/voidelle.h>
#include <interface/fs/voidelle/voidelle.h>
#include <interface/fio.h>
#include <lib/string.h>
#include <memory/memory.h>

const char *GetFilename(const char *path)
{
    const char *last = strrchr(path, '/');
    if (last)
        return last + 1;
    else
        return path;
}

bool FindParentVoidelleByPath(Voidom voidom, const char *path, Voidelle *buf)
{
    char *tmp = malloc(strlen(path) + 1);
    strcpy(tmp, path);

    char *slash = strrchr(tmp, '/');
    if (slash == tmp)
        slash[1] = '\0';
    else
        *slash = '\0';

    return FindVoidelleByPath(voidom, tmp, buf);
}

bool FindVoidelleByPath(Voidom voidom, const char *path, Voidelle *buf)
{
    if (path[0] != '/')
        return false;
    if (strcmp(path, "/") == 0)
    {
        read_void(voidom, &voidom.root, VOID_SIZE, sizeof(Voidelle));
        memcpy(buf, &voidom.root, sizeof(Voidelle));
        return true;
    }

    read_void(voidom, &voidom.root, VOID_SIZE, sizeof(Voidelle));
    Voidelle parent = voidom.root;

    const char *beginning = path + 1;

    while (true)
    {
        const char *end = beginning;
        while (*end != '\0' && *end != '/')
            end++;

        unsigned long filename_len = end - beginning;

        char *filename = malloc(filename_len + 1);
        memcpy(filename, beginning, filename_len);
        filename[filename_len] = 0;

        uint64_t child_pos = parent.content_voidelle;
        while (child_pos)
        {
            Voidelle child;
            read_void(voidom, &child, child_pos, sizeof(Voidelle));

            char vname[child.name_voidelle_size];
            get_voidelle_name(voidom, child, vname);

            if (strcmp(vname, filename) == 0)
            {
                parent = child;
                break;
            }

            child_pos = child.next_voidelle;
        }

        if (child_pos == 0)
            return false;

        if (*end == '\0')
            break;

        beginning = end + 1;
    }

    if (buf != 0)
        memcpy(buf, &parent, sizeof(Voidelle));
    return true;
}

FilesystemInterface GetVoidelleInterface(Voidom *voidom)
{
    FilesystemInterface interface;
    interface.fread = VoidelleIRead;
    interface.fopen = 0;
    interface.fwrite = VoidelleIWrite;
    interface.fcreate_directory = VoidelleICreateDirectory;
    interface.fcreate_file = VoidelleICreateFile;
    interface.key = voidom;

    return interface;
}