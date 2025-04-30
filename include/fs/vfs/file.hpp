#ifndef FILE_HPP
#define FILE_HPP

#include <fs/fat32.hpp>
#include <fs/vfs/vfsnode.hpp>

namespace veil
{
    class File : public VFSNode
    {

    public:
        unsigned char *GetContent()
        {
            return fs->ReadFile(&this->entry);
        }

        void Write(unsigned char *buf, unsigned long size)
        {
            fs->WriteToEntry(&this->entry, buf, size);
        }

        void Delete()
        {
            fs->DeleteEntry(&this->entry, false);
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
            : VFSNode(fs, name, entry)
        {
        }

        File(FatFS *fs, FAT32DirectoryEntry entry, const char *name)
            : VFSNode(fs, (const unsigned char *)name, entry)
        {
        }

        void Rename(const char *new_name);
        static File *Open(const char *dir);
        static void Close(File *&file);
        static bool Exists(const char *dir);
        static File *Create(const char *dir);
    };
}

#endif