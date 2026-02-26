#include <elf.h>
#include <memory/mmu.h>
#include <memory/memory.h>
#include <lib/printf.h>
#include <scheduler/task.h>
#include <scheduler/scheduler.h>
#include <interface/fio.h>
#include <lib/string.h>
#include <interface/errno.h>

Task *MakeElfProcess(const char *path, bool kernel, PID pid, int argc, char **argv)
{
    FILEHANDLE file = OpenFile(FILE_READ, path);
    if (file == -1)
        return 0;

    Elf64_Ehdr eheader;
    if (ReadFile(file, &eheader, sizeof(eheader), 0) == -E_NO_FILE)
    {
        LOG("ELF file does not exist.\n");
        CloseFile(file);
        return 0;
    }

    if (memcmp(eheader.e_ident, ELFMAG, 4) != 0)
    {
        LOG("ELF file provided is not of the corresponding type (%c %c %c %c).\n",
            eheader.e_ident[0],
            eheader.e_ident[1],
            eheader.e_ident[2],
            eheader.e_ident[3]);
        CloseFile(file);
        return 0;
    }

    if (eheader.e_machine != EM_AARCH64)
    {
        LOG("ELF file is not of type AARCH64.\n");
        CloseFile(file);
        return 0;
    }

    Task *task = CreateTask(path, kernel, eheader.e_entry, 0, argc, argv);
    char user = kernel ? 0 : MMU_USER;
    char exec = kernel ? 0 : MMU_USER_EXEC;
    char rw = kernel ? MMU_NORW : MMU_RWRW;

    for (unsigned long i = 0; i < eheader.e_phnum; i++)
    {
        Elf64_Phdr phdr;
        ReadFile(file, &phdr, sizeof(phdr), eheader.e_phoff + i * eheader.e_phentsize);

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

        ReadFile(file, read, phdr_filesz, phdr_offset);

        MapTaskPage(task, phdr_vaddr, VIRT_TO_PHYS(read), phdr_memsz, user | exec | rw);
    }

    CloseFile(file);

    if (pid == -1)
        pid = AddTask(task);

    task->pid = pid;
    return task;
}