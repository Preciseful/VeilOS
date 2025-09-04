#include <elf.h>
#include <memory/mmu.h>
#include <memory/memory.h>
#include <lib/printf.h>
#include <scheduler/task.h>
#include <scheduler/scheduler.h>

Task *MakeElfProcess(const char *path)
{
    // VNode *vnode = OpenFile(path);
    // if (!vnode)
    //     return 0;

    // Elf64_Ehdr eheader;
    // SeekInFile(vnode, 0, SEEK_SET);
    // ReadFile(&eheader, sizeof(eheader), vnode);

    // if (memcmp(eheader.e_ident, ELFMAG, 4) != 0)
    // {
    //     LOG("ELF file provided is not of the corresponding type.\n");
    //     return 0;
    // }

    // if (eheader.e_machine != EM_AARCH64)
    // {
    //     LOG("ELF file is not of type AARCH64.\n");
    //     return 0;
    // }

    // Task *task = CreateTask(path, eheader.e_entry, 0);

    // for (unsigned long i = 0; i < eheader.e_phnum; i++)
    // {
    //     Elf64_Phdr phdr;
    //     SeekInFile(vnode, eheader.e_phoff + i * eheader.e_phentsize, SEEK_SET);
    //     ReadFile(&phdr, sizeof(phdr), vnode);

    //     if (phdr.p_type != PT_LOAD)
    //         continue;

    //     // align down both of these to page
    //     unsigned long phdr_offset = (phdr.p_offset / PAGE_SIZE) * PAGE_SIZE;
    //     unsigned long phdr_vaddr = (phdr.p_vaddr / PAGE_SIZE) * PAGE_SIZE;

    //     // pad the two
    //     unsigned long pad_front = phdr.p_offset - phdr_offset;
    //     unsigned long phdr_filesz = phdr.p_filesz + pad_front;
    //     unsigned long phdr_memsz = phdr.p_memsz + pad_front;

    //     unsigned char *read = malloc(phdr_memsz);
    //     memset(read, 0, phdr_memsz);
    //     SeekInFile(vnode, phdr_offset, SEEK_SET);
    //     ReadFile(read, phdr_filesz, vnode);

    //     MapTaskPage(task, phdr_vaddr, MMU_USER_EXEC | MMU_RWRW, (VirtualAddr)read, phdr_memsz);
    // }

    // CloseFile(vnode);
    // free(vnode);

    // AddTask(task);

    // return task;

    return 0;
}