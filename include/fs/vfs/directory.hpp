#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <lib/list.hpp>
#include <fs/fat32.hpp>
#include <fs/vfs/vfsnode.hpp>
#include <fs/vfs/file.hpp>

namespace veil
{
    class Directory : public VFSNode
    {
        FAT32DirectoryEntry entry;
        veil::std::List<FAT32DirectoryEntry> entries;
        veil::std::List<Directory *> directories;
        veil::std::List<File *> files;

    public:
        Directory(FatFS *fs, FAT32DirectoryEntry entry, veil::std::List<FAT32DirectoryEntry> entries, const unsigned char *name);
        Directory(FatFS *fs, FAT32DirectoryEntry entry, veil::std::List<FAT32DirectoryEntry> entries, const char *name);

        veil::std::List<Directory *> GetDirectories();
        veil::std::List<File *> GetFiles();
        veil::std::List<VFSNode *> GetTotalNodes();

        unsigned char Attributes()
        {
            return entry.Attributes();
        }
    };
}

#endif