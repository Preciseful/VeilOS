#include <fs/vfs/directory.hpp>
#include <fs/vfs/fscashier.hpp>

using namespace veil;

Directory::Directory(FatFS *fs, FAT32DirectoryEntry entry, veil::std::List<FAT32DirectoryEntry> entries, const unsigned char *name)
    : VFSNode(fs, name),
      entry(entry),
      entries(entries),
      directories(entries.Count(), entries.Capacity()),
      files(entries.Count(), entries.Capacity())
{
    FSCashier::GetCashier()->Add(this);
}

Directory::Directory(FatFS *fs, FAT32DirectoryEntry entry, veil::std::List<FAT32DirectoryEntry> entries, const char *name)
    : VFSNode(fs, (const unsigned char *)name),
      entry(entry),
      entries(entries),
      directories(entries.Count(), entries.Capacity()),
      files(entries.Count(), entries.Capacity())
{
    FSCashier::GetCashier()->Add(this);
}

veil::std::List<Directory *> Directory::GetDirectories()
{
    veil::std::List<Directory *> ret;

    printf("Reading with %lu entries...\n", entries.Count());
    for (unsigned long i = 0; i < entries.Count(); i++)
    {
        // skip over "." and ".."
        printf("%lu entry: %s\n", i, entries[i].name);
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

        if (directories[i])
            FSCashier::GetCashier()->RefreshExpiration(directories[i]);
        else
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

        if (files[i])
            FSCashier::GetCashier()->RefreshExpiration(files[i]);
        else
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