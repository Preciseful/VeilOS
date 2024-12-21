#ifndef VFSNODE_HPP
#define VFSNODE_HPP

#include <fs/fat32.hpp>

namespace veil::fs::vfs
{
    class VFSNode
    {
        bool preserve = false;
        unsigned long interactions = 0;

    protected:
        FatFS *fs;
        const unsigned char *name;

        VFSNode(FatFS *fs, const unsigned char *name) : fs(fs), name(name) {}

    public:
        void Preserve()
        {
            this->preserve = true;
        }

        bool IsPreserved()
        {
            return preserve;
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