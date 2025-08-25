#include <drivers/uart.h>
#include <drivers/gic.h>
#include <boot/interrupts.h>
#include <lib/printf.h>
#include <drivers/timer.h>
#include <scheduler/task.h>
#include <scheduler/scheduler.h>
#include <fs/fat32.h>
#include <fs/voidelle.h>
#include <drivers/emmc.h>
#include <memory/memory.h>
#include <memory/mmu.h>
#include <lib/string.h>
#include <vfs/vfs.h>
#include <vfs/vnode.h>
#include <bundles/elf.h>
#include <syscall/syscall.h>
#include <interface/portal.h>

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

    Partition *partitions = PartitionsInit();

    FatFS *fatfs = FatFSInit(partitions[0]);
    Voidom *voidom = VoidelleInit(partitions[1]);

    VfsInit();
    AddRootToVfs(fatfs, FAT32, '@');
    AddRootToVfs(voidom, VOIDELLE, '/');

    LOG("VFS initialized with boot FAT32 and root VOIDELLE.\n");

    RegisterPortal(PORTAL_UART, UartPortalRead, UartPortalWrite, 0);

    MakeElfProcess("@kernel/modules/Luna.elf");

    VNode *node = OpenFile("/123");
    SeekInFile(node, 0, SEEK_SET);

    LOG("Node: %s\n", node->path);
    char *read = malloc(VOIDITE_CONTENT_SIZE * 5);
    unsigned long read_count = ReadFile(read, VOIDITE_CONTENT_SIZE * 5, node);
    read[read_count] = 0;

    LOG("read : \n%s\n", read);

    while (1)
        Schedule();
}