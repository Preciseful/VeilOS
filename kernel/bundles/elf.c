#include <elf.h>
#include <memory/mmu.h>
#include <memory/memory.h>
#include <lib/printf.h>
#include <scheduler/task.h>
#include <scheduler/scheduler.h>
#include <vfs/vfs.h>

Task *MakeElfProcess(const char *path, bool kernel, int argc, char **argv, char **environment, long pid)
{
    FileID file = OpenFile(path);
    if (file == -1)
        return 0;

    Elf64_Ehdr eheader;
    SeekInFile(file, 0);
    ReadFromFile(file, &eheader, sizeof(eheader));

    if (memcmp(eheader.e_ident, ELFMAG, 4) != 0)
    {
        LOG("ELF file provided is not of the corresponding type.\n");
        return 0;
    }

    if (eheader.e_machine != EM_AARCH64)
    {
        LOG("ELF file is not of type AARCH64.\n");
        return 0;
    }

    Task *task = CreateTask(path, kernel, eheader.e_entry, 0, environment, argv, argc);
    char user = kernel ? 0 : MMU_USER;
    char exec = kernel ? 0 : MMU_USER_EXEC;
    char rw = kernel ? MMU_NORW : MMU_RWRW;

    for (unsigned long i = 0; i < eheader.e_phnum; i++)
    {
        Elf64_Phdr phdr;
        SeekInFile(file, eheader.e_phoff + i * eheader.e_phentsize);
        ReadFromFile(file, &phdr, sizeof(phdr));

        if (phdr.p_type != PT_LOAD)
            continue;

        // align down both of these to page
        unsigned long phdr_offset = (phdr.p_offset / PAGE_SIZE) * PAGE_SIZE;
        unsigned long phdr_vaddr = (phdr.p_vaddr / PAGE_SIZE) * PAGE_SIZE;

        // pad the two
        unsigned long pad_front = phdr.p_offset - phdr_offset;
        unsigned long phdr_filesz = phdr.p_filesz + pad_front;
        unsigned long phdr_memsz = phdr.p_memsz + pad_front;

        unsigned char *read = malloc(phdr_memsz);
        memset(read, 0, phdr_memsz);
        SeekInFile(file, phdr_offset);
        ReadFromFile(file, read, phdr_filesz);

        MapTaskPage(task, phdr_vaddr, user | exec | rw, (VirtualAddr)read, phdr_memsz, MAP_PROPERTY_CODE);
    }

    CloseFile(file);

    if (pid == -1)
        pid = AddTask(task);

    task->pid = pid;
    return task;
}