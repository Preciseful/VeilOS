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

    int x = 2021;
    printf("how to move on from %d;\n", x);

    set_vtable();

    timer_init();
    gic_allow(30, 0);
    gic_allow(153, 0);
    irq_enable();

    scheduler_init();

    emmc_init();

    fatfs_t *fatfs = fatfs_init();
    fatfs_node_t *nodes;
    unsigned long nodes_count = get_fatentries(fatfs, fatfs->root_cluster, &nodes);

    for (unsigned long i = 0; i < nodes_count; i++)
    {
        printf("file: %s\n", nodes[i].name);
        if (strcmp(nodes[i].name, "config.txt") == 0)
        {
            unsigned char *data = read_fatnode(nodes[i]);
            printf("%s\n", data);
        }
    }

    printf("read all fat32\n");
    vfs_init();
    printf("vfs init\n");
    add_root(fatfs, FAT32, '/');
    printf("root\n");

    make_elf_process("/modules/Luna.elf");

    while (1)
    {
        schedule();
    }
}