#ifndef FILE_HPP
#define FILE_HPP

#include <fs/fat32.hpp>
#include <fs/vfs/vfsnode.hpp>
#include <fs/vfs/fscashier.hpp>

namespace veil
{
    class File : public VFSNode
    {
        FAT32DirectoryEntry entry;

    public:
        unsigned char *GetContent()
        {
            return fs->ReadFile(&this->entry);
        }

        unsigned char Attributes()
        {
            return entry.Attributes();
        }

        unsigned long Size()
        {
            return entry.Size();
        }

        File(FatFS *fs, FAT32DirectoryEntry entry, const unsigned char *name)
            : VFSNode(fs, name), entry(entry)
        {
            FSCashier::GetCashier()->Add(this);
        }

        File(FatFS *fs, FAT32DirectoryEntry entry, const char *name)
            : VFSNode(fs, (const unsigned char *)name), entry(entry)
        {
            FSCashier::GetCashier()->Add(this);
        }

        static File *Find(const char *dir);
    };
}

#endif