#include <drivers/uart.h>
#include <drivers/gic.h>
#include <boot/interrupts.h>
#include <lib/printf.h>
#include <drivers/timer.h>
#include <scheduler/process.h>
#include <fs/fat32.h>
#include <drivers/emmc.h>
#include <memory/memory.h>
#include <lib/string.h>

void putc(void *p, char c)
{
    uart_put(c);
}

void pr0()
{
    while (1)
    {
        printf("0");
    }
}

void pr1()
{
    while (1)
    {
        printf("1");
    }
}

void kmain()
{
    uart_init();
    init_printf(0, putc);

    uart_puts("health 2021\n");
    mm_init();

    int x = 2021;
    printf("how to move on from %d;", x);

    set_vtable();
    timer_init();
    gic_allow(30, 0);
    gic_allow(153, 0);
    irq_enable();

    scheduler_init();

    pcreate(&pr0);
    pcreate(&pr1);

    emmc_init();
    printf("b");
    fatfs_t *fatfs = fatfs_init();
    printf("d");
    fatfs_node_t *nodes;
    unsigned long nodes_count = get_fatentries(fatfs, fatfs->root_cluster, &nodes);
    printf("c");

    for (unsigned long i = 0; i < nodes_count; i++)
    {
        printf("file: %s\n", nodes[i].name);
        if (strcmp(nodes[i].name, "config.txt") == 0)
        {
            unsigned char *data = read_fatnode(nodes[i]);
            printf("%s\n", data);
        }
    }

    while (1)
    {
        schedule();
    }
}