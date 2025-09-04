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
#include <interface/portal.h>
#include <fs/voidelle.h>
#include <vfs/vfs.h>

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

    PortalsInit();
    RegisterPortal(PORTAL_UART, 0, UartPortalRead, UartPortalWrite, 0);
    LOG("Initialized UART portal.\n");

    EmmcInit();

    Partition *partitions = PartitionsInit();

    FatFS *fatfs = malloc(sizeof(FatFS));
    Voidom *voidom = malloc(sizeof(Voidom));
    Portal voidelle_portal;

    FatFSInit(fatfs, partitions[0]);
    VoidelleInit(voidom, &voidelle_portal, partitions[1]);
    LOG("Initialized partitions.\n");

    VFSInit();
    LOG("VFS initialized.\n");

    AddRoot("/", voidelle_portal);
    LOG("Added root.\n");

    LOG("Creating process Luna.elf...\n");
    MakeElfProcess("/Luna.elf");
    LOG("Created Luna.elf.\n");

    while (1)
        Schedule();
}