#include <lib/elf.hpp>
#include <elf.h>

using namespace veil;

#define Page(expr) ((expr) & ~0xFFF)

void apply_relocation(unsigned long *fixup_addr, unsigned long type,
                      unsigned long load_address, unsigned long offset,
                      long addend)
{
    unsigned long A = addend;
    unsigned long P = load_address + offset;
    unsigned long S = load_address;
    unsigned long X = 0;

    switch (type)
    {
    case R_AARCH64_RELATIVE:
    case R_AARCH64_ABS64:
    {
        if ((unsigned long)fixup_addr & 7 == 0)
            *(unsigned long *)fixup_addr = S + A;
        else
        {
            unsigned long x = S + A;
            memcpy(fixup_addr, &x, sizeof(unsigned long));
        }
        break;
    }

    case R_AARCH64_ABS32:
    {
        if ((unsigned long)fixup_addr & 3 == 0)
            *(unsigned int *)fixup_addr = (S + A) & 0xFFFFFFFF;
        else
        {
            unsigned int x = (S + A) & 0xFFFFFFFF;
            memcpy((unsigned int *)fixup_addr, &x, sizeof(unsigned int));
        }
        break;
    }

    case R_AARCH64_ABS16:
    {
        *(unsigned short *)fixup_addr = (S + A) & 0xFFFF;
        break;
    }

    case R_AARCH64_PREL64:
    {
        *(unsigned long *)fixup_addr = S + A - P;
        break;
    }

    case R_AARCH64_PREL32:
    {
        *(unsigned int *)fixup_addr = (S + A - P) & 0xFFFFFFFF;
        break;
    }

    case R_AARCH64_PREL16:
    {
        *(unsigned short *)fixup_addr = (S + A - P) & 0xFFFF;
        break;
    }

    case R_AARCH64_CALL26:
    case R_AARCH64_JUMP26:
    {
        long X = (S + A - P) / 4;

        unsigned int instr = *(unsigned int *)fixup_addr;
        instr &= ~(0x3FFFFFF << 0);
        instr |= (X & 0x3FFFFFF) << 0;
        *(unsigned int *)fixup_addr = instr;
        break;
    }

    case R_AARCH64_ADR_PREL_PG_HI21:
    {
        unsigned long page_S = Page(S + A);
        unsigned long page_P = Page(P);

        long delta = (long)(page_S - page_P);

        unsigned int imm21 = (unsigned int)((delta >> 12) & 0x1FFFFF);
        unsigned int instruction = *(unsigned int *)fixup_addr;

        // Clear the immediate field in the instruction
        instruction &= 0x9F00001F;
        instruction |= ((imm21 & 0x3) << 29) | ((imm21 & 0x1FFFFC) << 3);
        *(unsigned int *)fixup_addr = instruction;
        break;
    }

    case R_AARCH64_ADD_ABS_LO12_NC:
    {
        unsigned short imm12 = S & 0xFFF;

        *(unsigned int *)fixup_addr &= 0xFFC003FF;
        *(unsigned int *)fixup_addr |= (imm12 << 10);
        break;
    }
    default:
        printf("Unhandled relocation type: %lu\n", type);
        break;
    }
}

bool ELF::Initialize()
{
    printf("Testing relocatable bin...\n");
    unsigned char *content = file->GetContent();

    if (content[0] != 0x7F)
    {
        printf("File does not start with 0x7F!\n");
        return false;
    }

    if (!(content[1] == 'E' && content[2] == 'L' && content[3] == 'F'))
    {
        printf("File does not continue with ELF!\n");
        return false;
    }

    auto header = *(Elf64_Ehdr *)content;
    printf("Running rbin version %u\n", header.e_version);

    Elf64_Shdr *section_table = (Elf64_Shdr *)(content + header.e_shoff);

    char *sh_str = (char *)(content + section_table[header.e_shstrndx].sh_offset);
    printf("HI WORLD %d\n", header.e_shnum);
    for (int i = 0; i < header.e_shnum; i++)
    {
        printf("i: %d header->e_shnum: %d\n", i, header.e_shnum);
        if (section_table[i].sh_type == SHT_RELA)
        {
            Elf64_Rela *rela_entries = (Elf64_Rela *)(content + section_table[i].sh_offset);
            unsigned long entries = section_table[i].sh_size / section_table[i].sh_entsize;
            for (unsigned long j = 0; j < entries; j++)
            {
                unsigned long *fixup_addr = (unsigned long *)(content + rela_entries[j].r_offset);
                apply_relocation(fixup_addr, ELF64_R_TYPE(rela_entries[j].r_info), (unsigned long)content, rela_entries[j].r_offset, rela_entries[j].r_addend);
            }
        }
    }

    Elf64_Addr min_vaddr = 9999999;
    int i_smallest = 0;

    Elf64_Phdr *program_table = (Elf64_Phdr *)(content + header.e_phoff);
    for (int i = 0; i < header.e_phnum; i++)
    {
        if (program_table[i].p_type == PT_NULL)
            continue;

        if (program_table[i].p_vaddr < min_vaddr)
        {
            min_vaddr = program_table[i].p_vaddr;
            i_smallest = i;
        }
    }

    unsigned long load_segment_addr = (unsigned long)(content + program_table[i_smallest].p_offset);
    unsigned long load_bias = load_segment_addr - min_vaddr;
    unsigned char *entry_ptr = (unsigned char *)(load_bias + header.e_entry);

    printf("entry: %lu\n", content + header.e_entry);
    Entry = (void (*)())entry_ptr;

    return true;
}