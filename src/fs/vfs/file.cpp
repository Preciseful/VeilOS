#include <fs/vfs/file.hpp>
#include <fs/vfs/directory.hpp>
#include <fs/vfs/vfs.h>
#include <lib/string.hpp>
#include <fs/vfs/city.hpp>
using namespace veil;

File *File::Open(const char *dir)
{
    char temp[100] = "";
    unsigned long i = 0;
    if (dir[0] == '/' || dir[0] == '\\')
        dir++;
    else
    {
        printf("Invalid directory name '%s'! Must be absolute.\n", dir);
        return 0;
    }

    City *current = root_city;

    while (*dir)
    {
        if (*dir == '/' || *dir == '\\')
        {
            current = current->GetSubcity(rtrim((unsigned char *)temp));

            if (!current)
            {
                printf("Cannot find subdirectory '%s'\n", temp);
                return 0;
            }

            i = 0;
        }
        else
        {
            temp[i] = *dir;
            temp[i + 1] = '\0';
            i++;
        }

        dir++;
    }
    auto filename = rtrim((unsigned char *)temp);

    auto filecity = current->GetSubcity(filename);
    if (filecity->Attributes & 0x2)
    {
        printf("File '%s' is already open!\n", filename);
        return nullptr;
    }

    filecity->Attributes |= 0x2;

    auto file = current->GetFile(filename);
    if (!file)
    {
        printf("Cannot find file '%s'\n", temp);
        filecity->Attributes &= ~0x2;
    }

    return file;
}

unsigned long fileFind(char *path)
{
    return (unsigned long)File::Open(path);
}