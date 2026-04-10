/**
 * @author Developful
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <interface/fio.h>
#include <elf.h>
#include <interface/errno.h>
#include <lib/printf.h>

typedef struct KernelSymbols
{
    Elf64_Sym *table;
    unsigned long table_size;
    char *names;
    unsigned long names_size;
} KernelSymbols;

KernelSymbols symbols = {0};

void CacheKernelSymbols()
{
    FILEHANDLE kernel = OpenFile(FILE_READ, "/kernel/kernel8.elf");
    if (kernel == -1)
    {
        LOG("No kernel8.elf file found for caching.\n");
        CloseFile(kernel);
        return;
    }

    Elf64_Ehdr eheader;
    if (ReadFile(kernel, &eheader, sizeof(eheader), 0) == -E_NO_FILE)
    {
        LOG("No kernel8.elf file found for caching.\n");
        CloseFile(kernel);
        return;
    }

    for (int i = 0; i < eheader.e_shnum; i++)
    {
        Elf64_Shdr symtab;
        ReadFile(kernel, &symtab, sizeof(symtab), eheader.e_shoff + i * eheader.e_shentsize);

        if (symtab.sh_type != SHT_SYMTAB)
            continue;

        Elf64_Shdr strtab;
        ReadFile(kernel, &strtab, sizeof(strtab), eheader.e_shoff + symtab.sh_link * eheader.e_shentsize);

        symbols.table = malloc(symtab.sh_size);
        symbols.table_size = symtab.sh_size / sizeof(Elf64_Sym);
        ReadFile(kernel, symbols.table, symtab.sh_size, symtab.sh_offset);

        symbols.names = malloc(strtab.sh_size);
        symbols.names_size = strtab.sh_size;
        ReadFile(kernel, symbols.names, strtab.sh_size, strtab.sh_offset);

        break;
    }

    CloseFile(kernel);
}

void print_best_symbol(unsigned long addr)
{
    Elf64_Sym best_sym;
    best_sym.st_name = 0;

    if (symbols.table == 0)
    {
        Printf("0x%lx", addr);
        return;
    }

    for (int i = 0; i < symbols.table_size; i++)
    {
        Elf64_Sym sym = symbols.table[i];
        if (sym.st_name >= symbols.names_size)
            continue;

        if (sym.st_value <= addr && sym.st_value > best_sym.st_value)
            best_sym = sym;
    }

    Printf("%s", &symbols.names[best_sym.st_name]);
}

void Trace(unsigned long x29)
{
    unsigned long *fp = (unsigned long *)x29;

    while (fp)
    {
        print_best_symbol(fp[1] - 1);
        fp = (unsigned long *)fp[0];

        if (fp[0] < HIGH_VA)
            return;

        if (fp)
            Printf(" <- ");
    }
}
