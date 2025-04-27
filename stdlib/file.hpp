#ifndef FILE_HPP
#define FILE_HPP

namespace veil
{
    class File
    {
    public:
        unsigned char *GetContent();
        static File *Find(const char *dir);
    };
}

#endif