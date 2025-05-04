#include <fs/vfs/directory.hpp>
#include <fs/vfs/city.hpp>
#include <fs/vfs/vfs.h>

using namespace veil;

Directory::Directory(FatFS *fs, FAT32DirectoryEntry entry, veil::std::List<FAT32DirectoryEntry> entries, const unsigned char *name)
    : VFSNode(fs, name, entry),
      entries(entries),
      directories(entries.Count(), entries.Capacity()),
      files(entries.Count(), entries.Capacity())
{
}

Directory::Directory(FatFS *fs, FAT32DirectoryEntry entry, veil::std::List<FAT32DirectoryEntry> entries, const char *name)
    : VFSNode(fs, (const unsigned char *)name, entry),
      entries(entries),
      directories(entries.Count(), entries.Capacity()),
      files(entries.Count(), entries.Capacity())
{
}

veil::std::List<Directory *> Directory::GetDirectories()
{
    veil::std::List<Directory *> ret;

    //  printf("Reading %s with %lu entries...\n", this->Name(), entries.Count());
    for (unsigned long i = 0; i < entries.Count(); i++)
    {
        // skip over "." and ".."
        // printf("%lu entry: %s\n", i, entries[i].name);
        if (i == 0 && entries[i].Attributes() & 0x10)
        {
            i++;
            continue;
        }

        // skip over current volume
        if (i == 0 && entries[i].Attributes() & 0x08)
            continue;

        if (!(entries[i].Attributes() & 0x10))
            continue;

        directories[i] = new Directory(fs, fs->GetEntry(entries[i].cluster), fs->GetEntries(entries[i].cluster), entries[i].name);
        ret.Add(directories[i]);
    }

    return ret;
}

veil::std::List<File *> Directory::GetFiles()
{
    veil::std::List<File *> ret;

    for (unsigned long i = 0; i < entries.Count(); i++)
    {
        if (entries[i].Attributes() & 0x10 || entries[i].Attributes() & 0x08)
            continue;

        files[i] = new File(fs, entries[i], entries[i].name);
        ret.Add(files[i]);
    }

    return ret;
}

veil::std::List<VFSNode *> Directory::GetTotalNodes()
{
    veil::std::List<VFSNode *> stuff = GetDirectories().Convert<VFSNode *>();
    stuff.AddRange(GetFiles().Convert<VFSNode *>());
    return stuff;
}

bool Directory::Exists(const char *dir)
{
    char temp[100] = "";
    unsigned long i = 0;
    if (dir[0] == '/')
    {
        if (dir[1] == '\0')
            return true;
        dir++;
    }
    else
    {
        ERROR("Invalid directory name '%s'! Must be absolute.\n", dir);
        return false;
    }

    City *current = root_city;

    while (*dir)
    {
        if (*dir == '/')
        {
            current = current->GetSubcity(rtrim((unsigned char *)temp));

            if (!current)
            {
                ERROR("Cannot find subdirectory '%s'!\n", temp);
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

    if (i != 0)
    {
        auto dirname = rtrim((unsigned char *)temp);
        auto file = current->GetDirectory(dirname);
        if (!file)
        {
            ERROR("Cannot find directory '%s'!\n", temp);
            return false;
        }

        return true;
    }
    else
        return current->Attributes & DirectoryType;
}

Directory *Directory::Create(const char *dir)
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

            if (*(dir + 1) == '\0')
            {
                if (current)
                {
                    ERROR("Directory '%s' already exists!\n", temp);
                    return 0;
                }
                else
                {
                    ERROR("Cannot find subdirectory '%s'!\n", temp);
                    return 0;
                }
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

    auto dirname = rtrim((unsigned char *)temp);
    auto dircity = current->GetSubcity(dirname);
    if (dircity)
    {
        ERROR("Directory %s exists already!\n", dirname);
        return 0;
    }

    auto entry = current->fs->WriteNewEntry(current, current->GetCluster(), (char *)dirname, 0x10);
    dircity = new City(current, dirname, FileType, entry, current->fs);
    current->AddSubcity(dircity);

    auto dirfile = new Directory(current->fs, entry, veil::std::List<FAT32DirectoryEntry>(), dirname);
    dirfile->OwnCity = dircity;
    return dirfile;
}