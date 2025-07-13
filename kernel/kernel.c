#include <drivers/uart.h>
#include <drivers/gic.h>
#include <boot/interrupts.h>
#include <lib/printf.h>
#include <drivers/timer.h>
#include <scheduler/process.h>
#include <fs/fat32.h>
#include <fs/voidelle.h>
#include <drivers/emmc.h>
#include <memory/memory.h>
#include <memory/mmu.h>
#include <lib/string.h>
#include <vfs/vfs.h>
#include <vfs/vnode.h>

__attribute__((aligned(4096))) void pr0()
{
    unsigned int i = 0;
    while (1)
    {
        i++;
        long fr = (PERIPHERAL_BASE + 0x201000 + 0x18);
        while ((*((volatile unsigned int *)fr)) & (1 << 5))
            ;
        long dr = (PERIPHERAL_BASE + 0x201000);
        *((volatile unsigned int *)dr) = '!';
    }
}

__attribute__((aligned(4096))) void pr1()
{
    while (1)
    {
        printf("1");
    }
}

void kmain()
{
    uart_init();
    uart_puts("health 2021\n");
    mm_init();
    mmu_init();

    int x = 2021;
    printf("how to move on from %d;\n", x);

    set_vtable();
    timer_init();
    gic_allow(30, 0);
    gic_allow(153, 0);
    irq_enable();

    scheduler_init();

    pcreate((unsigned long)&pr0, 0x0, false);
    pcreate((unsigned long)&pr1, 0x0, false);

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

    vfs_init();
    add_root(fatfs, FAT32, '/');
    vnode_t *vnode = fopen("/modules/Luna.elf");
    printf("opened: %s\n", ((fatfs_node_t *)vnode->data)->name);

    while (1)
    {
        schedule();
    }
}