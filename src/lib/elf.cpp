#include <lib/elf.hpp>
#include <elf.h>
#include <lib/string.h>
#include <drivers/miniuart.h>
#include <lib/fork.h>

using namespace veil;

#define Page(expr) ((expr) & ~0xFFF)

struct ELF::kpatch patches[] = {
    {"File", "GetContent", (unsigned long)(void *)&File::GetContent},
    {"File", "Size", (unsigned long)(void *)&File::Size},
    {"File", "5Write", (unsigned long)(void *)&File::Write},
    {"File", "9WriteText", (unsigned long)(void *)&File::WriteText},
    {"File", "Delete", (unsigned long)(void *)&File::Delete},
    {"File", "Rename", (unsigned long)(void *)&File::Rename},
    {"File", "Open", (unsigned long)&File::Open},
    {"File", "Close", (unsigned long)&File::Close},
    {"File", "Exists", (unsigned long)&File::Exists},
    {"File", "Create", (unsigned long)&File::Create}};

void apply_relocation(unsigned long *fixup_addr, unsigned long type,
                      unsigned long load_address, unsigned long offset,
                      long addend)
{
    unsigned long A = addend;
    unsigned long P = load_address + offset;
    unsigned long S = load_address;

    switch (type)
    {
    case R_AARCH64_JUMP_SLOT:
    case R_AARCH64_RELATIVE:
    case R_AARCH64_ABS64:
    {
        if (((unsigned long)fixup_addr & 7) == 0)
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
        ERROR("Unhandled relocation type: %lu\n", type);
        break;
    }
}

void flush_instruction_cache(void *start, void *end)
{
    uintptr_t addr = (uintptr_t)start;
    while (addr < (uintptr_t)end)
    {
        asm volatile("dc cvau, %0" : : "r"(addr));
        addr += 64;
    }
    asm volatile("dsb ish");
    addr = (uintptr_t)start;
    while (addr < (uintptr_t)end)
    {
        asm volatile("ic ivau, %0" : : "r"(addr));
        addr += 64;
    }
    asm volatile("dsb ish");
    asm volatile("isb");
}

unsigned long find_patch(unsigned char *name)
{
    for (auto patch : patches)
    {
        if (strcontains((unsigned char *)patch.first, name) && strcontains((unsigned char *)patch.second, name))
            return patch.func;
    }

    ERROR("No patch found for %s.\n", name);
    return 0;
}

// ELF format cheatsheet (very helpful thank u whoever made it)
// https://gist.github.com/x0nu11byt3/bcb35c3de461e5fb66173071a2379779
bool ELF::Initialize()
{
    INFO("Verifying ELF file...\n");
    unsigned char *content = file->GetContent();

    if (content[0] != 0x7F)
    {
        ERROR("File does not start with 0x7F!\n");
        return false;
    }

    if (!(content[1] == 'E' && content[2] == 'L' && content[3] == 'F'))
    {
        ERROR("File does not continue with ELF!\n");
        return false;
    }

    this->base = valloc(file->Size());
    auto header = (Elf64_Ehdr *)content;

    if (header->e_machine != EM_AARCH64)
    {
        ERROR("ELF file is not of type AARCH64!\n");
        return false;
    }

    if (header->e_type != ET_DYN)
    {
        ERROR("DYN ELF files are the only supported format!\n");
        return false;
    }

    auto tphdr = (Elf64_Phdr *)(content + header->e_phoff);

    for (int i = 0; i < header->e_phnum; i++)
    {
        auto phdr = tphdr[i];

        if (phdr.p_type == PT_LOAD)
        {
            void *dest = (void *)(base + phdr.p_vaddr);
            void *src = content + phdr.p_offset;
            memcpy(dest, src, phdr.p_filesz);

            if (phdr.p_memsz > phdr.p_filesz)
                memzero((unsigned long)dest + phdr.p_filesz, phdr.p_memsz - phdr.p_filesz);
        }

        else if (phdr.p_type == PT_DYNAMIC)
        {
            Elf64_Dyn *dyn = (Elf64_Dyn *)(content + phdr.p_offset);
            unsigned long rela_addr = 0;
            unsigned long rela_size = 0;
            unsigned long rela_entsize = sizeof(Elf64_Rela);
            unsigned long symtab_addr = 0;
            unsigned long strtab_addr = 0;
            unsigned long pltrel_addr = 0;
            unsigned long pltrel_size = 0;
            unsigned long needed = 0;

            for (Elf64_Dyn *entry = dyn; entry->d_tag != DT_NULL; entry++)
            {
                switch (entry->d_tag)
                {
                case DT_RELA:
                    rela_addr = entry->d_un.d_ptr;
                    break;
                case DT_RELASZ:
                    rela_size = entry->d_un.d_val;
                    break;
                case DT_SYMTAB:
                    symtab_addr = entry->d_un.d_ptr;
                    break;
                case DT_STRTAB:
                    strtab_addr = entry->d_un.d_ptr;
                    break;
                case DT_JMPREL:
                    pltrel_addr = entry->d_un.d_ptr;
                    break;
                case DT_PLTRELSZ:
                    pltrel_size = entry->d_un.d_val;
                    break;
                case DT_NEEDED:
                    needed = entry->d_un.d_val;
                    break;
                }
            }

            Elf64_Sym *symtab = (Elf64_Sym *)(base + symtab_addr);
            unsigned char *strtab = (unsigned char *)(base + strtab_addr);

            if (needed != 0)
                WARN("Library '%s' required for %s!\n", strtab + needed, this->file->Name());

            if (pltrel_addr && pltrel_size)
            {
                Elf64_Rela *plt_entries = (Elf64_Rela *)(base + pltrel_addr);
                int plt_count = pltrel_size / rela_entsize;

                for (int i = 0; i < plt_count; i++)
                {
                    Elf64_Rela *r = &plt_entries[i];
                    unsigned long offset = r->r_offset;
                    unsigned long info = r->r_info;
                    unsigned long type = ELF64_R_TYPE(info);
                    unsigned long sym = ELF64_R_SYM(info);
                    long addend = r->r_addend;

                    unsigned long S = base;
                    if (sym != 0)
                    {
                        const unsigned char *symbol_name = strtab + symtab[sym].st_name;
                        if (unsigned long addr = find_patch((unsigned char *)symbol_name))
                        {
                            INFO("Patched symbol '%s'!\n", symbol_name);
                            symtab[sym].st_value = addr;
                            S = symtab[sym].st_value;
                        }
                        else
                            S = base + symtab[sym].st_value;
                    }

                    unsigned long *fixup_addr = (unsigned long *)(base + offset);
                    apply_relocation(fixup_addr, type, S, offset, addend);
                }
            }

            if (rela_addr && rela_size)
            {
                Elf64_Rela *rela_entries = (Elf64_Rela *)(base + rela_addr);
                int count = rela_size / rela_entsize;

                for (int i = 0; i < count; i++)
                {
                    Elf64_Rela *r = &rela_entries[i];

                    unsigned long offset = r->r_offset;
                    unsigned long info = r->r_info;
                    unsigned long type = ELF64_R_TYPE(info);
                    unsigned long sym = ELF64_R_SYM(info);
                    long addend = r->r_addend;

                    unsigned long S = base;
                    if (type == R_AARCH64_GLOB_DAT || type == R_AARCH64_JUMP_SLOT || type == R_AARCH64_PREL32)
                    {
                        if (sym != 0)
                        {
                            S = base + symtab[sym].st_value;
                        }
                    }

                    unsigned long *fixup_addr = (unsigned long *)(base + offset);

                    apply_relocation(fixup_addr, type, S, offset, addend);
                }
            }
        }
    }

    Entry = (int (*)())(base + header->e_entry);

    return true;
}