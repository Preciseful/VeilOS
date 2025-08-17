#include <vfs/vnode.h>
#include <elf.h>
#include <memory/mmu.h>
#include <memory/memory.h>
#include <lib/printf.h>
#include <scheduler/process.h>

void make_elf_process(const char *path)
{
    vnode_t *vnode = fopen(path);

    Elf64_Ehdr eheader;
    fseek(vnode, 0, SEEK_SET);
    fread(&eheader, sizeof(eheader), vnode);

    if (memcmp(eheader.e_ident, ELFMAG, 4) != 0)
    {
        LOG("ELF file provided is not of the corresponding type.\n");
        return;
    }

    if (eheader.e_machine != EM_AARCH64)
    {
        LOG("ELF file is not of type AARCH64.\n");
        return;
    }

    task_t *task = pcreate(eheader.e_entry, 0);

    for (unsigned long i = 0; i < eheader.e_phnum; i++)
    {
        Elf64_Phdr phdr;
        fseek(vnode, eheader.e_phoff + i * eheader.e_phentsize, SEEK_SET);
        fread(&phdr, sizeof(phdr), vnode);

        if (phdr.p_type != PT_LOAD)
            continue;

        unsigned char *read = malloc(phdr.p_memsz);
        memset(read, 0, phdr.p_memsz);
        fseek(vnode, phdr.p_offset, SEEK_SET);
        fread(read, phdr.p_filesz, vnode);

        map_task_page(task, phdr.p_vaddr, MMU_USER_EXEC | MMU_RWRW, read, phdr.p_memsz);
    }
}