#include <drivers/uart.h>
#include <drivers/gic.h>
#include <boot/interrupts.h>
#include <lib/printf.h>
#include <drivers/timer.h>
#include <scheduler/task.h>
#include <scheduler/scheduler.h>
#include <fs/fat32.h>
#include <drivers/emmc.h>
#include <memory/memory.h>
#include <memory/mmu.h>
#include <lib/string.h>
#include <vfs/vfs.h>
#include <vfs/vnode.h>
#include <bundles/elf.h>
#include <syscall/syscall.h>

void kboot()
{
    MMUInit();
}

void kmain()
{
    UartInit();
    Printf("\n----- VeilOS -----\n");
    LOG("UART initialized.\n");

    set_vtable();

    timer_init();
    AllowInterruptInGIC(30, 0);
    AllowInterruptInGIC(153, 0);
    irq_enable();

    LOG("Timer enabled.\n");

    SchedulerInit();
    LOG("Scheduler initialized.\n");

    EmmcInit();

    FatFS *fatfs = FatFSInit();
    VfsInit();
    AddRootToVfs(fatfs, FAT32, '/');

    LOG("VFS initialized with root FAT32.\n");

    MakeELFProcess("/modules/Luna.elf");

    while (1)
    {
        Schedule();
    }
}