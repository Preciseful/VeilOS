#include <drivers/uart.h>
#include <drivers/gic.h>
#include <boot/interrupts.h>
#include <lib/printf.h>
#include <drivers/timer.h>
#include <scheduler/process.h>
#include <fs/fat32.h>
#include <drivers/emmc.h>
#include <memory/memory.h>
#include <memory/mmu.h>
#include <lib/string.h>
#include <vfs/vfs.h>
#include <vfs/vnode.h>
#include <lib/elf.h>
#include <syscall/syscall.h>

void kboot()
{
    mmu_init();
}

void kmain()
{
    uart_init();
    printf("\n----- VeilOS -----\n");
    LOG("UART initialized.\n");

    set_vtable();

    timer_init();
    gic_allow(30, 0);
    gic_allow(153, 0);
    irq_enable();

    LOG("Timer enabled.\n");

    scheduler_init();
    LOG("Scheduler initialized.\n");

    emmc_init();

    fatfs_t *fatfs = fatfs_init();
    vfs_init();
    add_root(fatfs, FAT32, '/');

    LOG("VFS initialized with root FAT32.\n");

    make_elf_process("/modules/Luna.elf");

    while (1)
    {
        schedule();
    }
}