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
#include <bundles/elf.h>
#include <fs/voidelle.h>
#include <system/vfs.h>
#include <interface/fio.h>
#include <interface/fs/voidelle/voidelle.h>
#include <interface/fs/fat32/fat32.h>
#include <drivers/framebuffer.h>
#include <system/trace.h>

void kboot()
{
    MMUInit();
}

void kmain()
{
    UartInit();
    FramebufferInit();

    Printf("\n----- VeilOS -----\n");
    LOG("Console initialized.\n");

    set_vtable();

    timer_init();
    AllowInterruptInGIC(30, 0);
    AllowInterruptInGIC(153, 0);
    irq_enable();

    LOG("Timer enabled.\n");

    SchedulerInit();
    LOG("Scheduler initialized.\n");

    EmmcInit();

    Partition *partitions = PartitionsInit();

    FatFS *fatfs = malloc(sizeof(FatFS));
    Voidom *voidom = malloc(sizeof(Voidom));

    FatFSInit(fatfs, partitions[0]);
    VoidelleFSInit(voidom, partitions[1]);

    LOG("Initialized partitions.\n");

    VFSInit();
    AddMountPoint("/", GetVoidelleInterface(voidom));
    AddMountPoint("/kernel", GetFat32Interface(fatfs));

    // CacheKernelSymbols();
    // LOG("Kernel symbols cached.\n");

    LOG("Used memory (reference): %lu bytes.\n", GetMemoryUsed());

    char *envargv[6];
    envargv[0] = "TTY";
    envargv[1] = "FRAMEBUFFER";
    envargv[2] = "TTYNO";
    envargv[3] = "0";
    envargv[4] = "TTYPERM";
    envargv[5] = "OWQ";

    MakeElfProcess("/Luna.elf", false, -1, 0, 0, 6, envargv);
    LOG("Created shell.\n");

    while (1)
        Schedule();
}