#ifndef VFSNODE_HPP
#define VFSNODE_HPP

#include <fs/fat32.hpp>

namespace veil
{
    class VFSNode
    {
        unsigned long interactions = 0;

    protected:
        FatFS *fs;
        const unsigned char *name;
        FAT32DirectoryEntry entry;

        VFSNode(FatFS *fs, const unsigned char *name, FAT32DirectoryEntry entry) : fs(fs), name(name), entry(entry) {}

    public:
        FatFS *GetFS()
        {
            return fs;
        }

        FAT32DirectoryEntry GetEntry()
        {
            return entry;
        }

        unsigned long Interactions()
        {
            return interactions;
        }

        const unsigned char *Name()
        {
            return name;
        }

        virtual unsigned char Attributes()
        {
            return 0;
        }

        virtual ~VFSNode()
        {
            delete name;
        }
    };
}

#endif