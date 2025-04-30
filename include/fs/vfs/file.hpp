#ifndef FILE_HPP
#define FILE_HPP

#include <fs/fat32.hpp>
#include <fs/vfs/vfsnode.hpp>
#include <lib/string.h>

namespace veil
{
    class File : public VFSNode
    {

    public:
        unsigned char *GetContent()
        {
            return fs->ReadFile(&this->entry);
        }

        void Write(const char *buf, unsigned long size)
        {
            fs->WriteToEntry(&this->entry, buf, size);
        }

        void WriteText(const char *buf)
        {
            unsigned long size = strlen((const unsigned char *)buf);
            char *newbuf = new char[size];
            memcpy(newbuf, buf, size);

            fs->WriteToEntry(&this->entry, newbuf, size);

            delete[] newbuf;
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