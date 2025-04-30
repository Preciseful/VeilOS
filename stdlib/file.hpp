#ifndef FILE_HPP
#define FILE_HPP

namespace veil
{
    class File
    {
    public:
        unsigned char *GetContent();
        unsigned long Size();
        void Write(const char *buf, unsigned long size);
        void WriteText(const char *buf);
        void Delete();
        void Rename(const char *new_name);

        static File *Open(const char *dir);
        static void Close(File *&file);
        static bool Exists(const char *dir);
        static File *Create(const char *dir);
    };
}

#endif