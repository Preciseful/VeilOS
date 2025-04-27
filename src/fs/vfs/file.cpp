#include <fs/vfs/file.hpp>
#include <fs/vfs/directory.hpp>
#include <fs/vfs/vfs.h>
#include <lib/string.h>
#include <cxxabi.h>
using namespace veil;

File *File::Find(const char *dir)
{
    char temp[100] = "";
    unsigned long i = 0;
    if (dir[0] == '/' || dir[0] == '\\')
        dir++;
    else
        return 0;

    Directory *current = root_directory;

    while (*dir)
    {
        if (*dir == '/' || *dir == '\\')
        {
            auto subdirectories = current->GetDirectories();
            bool found = false;
            for (unsigned long i = 0; i < subdirectories.Count(); i++)
            {
                if (strcmp(rtrim(subdirectories[i]->Name()), rtrim((const unsigned char *)temp)) == 0)
                {
                    current = subdirectories[i];
                    found = true;
                    break;
                }
            }

            if (!found)
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

    auto files = current->GetFiles();

    for (unsigned long i = 0; i < files.Count(); i++)
    {
        if (strcmp(rtrim(files[i]->Name()), rtrim((const unsigned char *)temp)) == 0)
        {
            files[i]->Preserve();
            return files[i];
        }
    }

    printf("Cannot find file '%s'\n", temp);
    return 0;
}

unsigned long fileFind(char *path)
{
    return (unsigned long)File::Find(path);
}