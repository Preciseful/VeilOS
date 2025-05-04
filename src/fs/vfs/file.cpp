#include <fs/vfs/file.hpp>
#include <fs/vfs/directory.hpp>
#include <fs/vfs/vfs.h>
#include <lib/string.h>
#include <fs/vfs/city.hpp>
using namespace veil;

void File::Rename(const char *new_name)
{
    for (unsigned long i = 0; i < this->OwnCity->Parent->names.Count(); i++)
    {
        if (strcmp(this->OwnCity->Parent->names[i], this->OwnCity->name) == 0)
        {
            this->OwnCity->Parent->names[i] = (const unsigned char *)new_name;
            break;
        }
    }

    this->entry = fs->MoveEntry(this->OwnCity->Parent, this->entry, new_name);
    this->OwnCity->name = (const unsigned char *)new_name;
    this->name = (const unsigned char *)new_name;
}

void File::Close(File *&file)
{
    file->OwnCity->Attributes &= ~0x4;
    file = nullptr;
}

File *File::Create(const char *dir)
{
    char temp[100] = "";
    unsigned long i = 0;
    if (dir[0] == '/' || dir[0] == '\\')
        dir++;
    else
    {
        ERROR("Invalid directory name '%s'! Must be absolute.\n", dir);
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
                ERROR("Cannot find subdirectory '%s'\n", temp);
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
    if (filecity)
    {
        ERROR("File %s exists already!\n", filename);
        return 0;
    }

    auto entry = current->fs->WriteNewEntry(current, current->GetCluster(), (char *)filename, 0x20);
    filecity = new City(current, filename, FileType, entry, current->fs);
    current->AddSubcity(filecity);
    auto file = new File(current->fs, entry, filename);
    file->OwnCity = filecity;
    filecity->Attributes |= 0x4;
    return file;
}

File *File::Open(const char *dir)
{
    char temp[100] = "";
    unsigned long i = 0;
    if (dir[0] == '/')
        dir++;
    else
    {
        ERROR("Invalid directory name '%s'! Must be absolute.\n", dir);
        return 0;
    }

    City *current = root_city;

    while (*dir)
    {
        if (*dir == '/')
        {
            current = current->GetSubcity(rtrim((unsigned char *)temp));

            if (!current)
            {
                ERROR("Cannot find subdirectory '%s'\n", temp);
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
    if (filecity->Attributes & 0x4)
    {
        ERROR("File '%s' is already open!\n", filename);
        return nullptr;
    }

    filecity->Attributes |= 0x4;

    auto file = current->GetFile(filename);
    if (!file)
    {
        ERROR("Cannot find file '%s'\n", temp);
        filecity->Attributes &= ~0x4;
    }

    return file;
}

bool File::Exists(const char *dir)
{
    char temp[100] = "";
    unsigned long i = 0;
    if (dir[0] == '/' || dir[0] == '\\')
        dir++;
    else
    {
        ERROR("Invalid directory name '%s'! Must be absolute.\n", dir);
        return false;
    }

    City *current = root_city;

    while (*dir)
    {
        if (*dir == '/' || *dir == '\\')
        {
            current = current->GetSubcity(rtrim((unsigned char *)temp));

            if (!current)
            {
                ERROR("Cannot find subdirectory '%s'\n", temp);
                return false;
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
    auto file = current->GetFile(filename);
    if (!file)
    {
        ERROR("Cannot find file '%s'\n", temp);
        return false;
    }

    return true;
}