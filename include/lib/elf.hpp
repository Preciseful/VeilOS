#ifndef ELF_HPP
#define ELF_HPP

#include <fs/vfs/file.hpp>
#include <stdbool.h>

class ELF
{
private:
    veil::File *file;

    struct elf_header
    {
        unsigned char identity[16];
        unsigned short type;
        unsigned short instruction_set;
        unsigned int elf_version;
        unsigned long entry_offset;
        unsigned long header_offset;
        unsigned long section_offset;
        unsigned int flags;
        unsigned char header_size;
        unsigned char size_program_entry;
        unsigned char num_program_entries;
        unsigned char size_section_entry;
        unsigned char num_section_entries;
        unsigned char section_index;
    } __attribute__((packed));

public:
    struct kpatch
    {
        const char *first;
        const char *second;
        unsigned long func;
    };

    unsigned long base;

    int (*Entry)(void);
    bool Initialize();

    ELF(veil::File *file) : file(file) {}
};

#endif