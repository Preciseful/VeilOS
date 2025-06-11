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
#include <lib/string.h>
#include <vfs/vfs.h>
#include <vfs/vnode.h>

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
    printf("how to move on from %d;\n", x);

    set_vtable();
    timer_init();
    gic_allow(30, 0);
    gic_allow(153, 0);
    irq_enable();

    scheduler_init();

    pcreate(&pr0);
    pcreate(&pr1);

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

    voidom_t *vdom = voidelle_init();
    voidelle_t *pony = create_voidelle(vdom, vdom->root->pos, VOIDELLE_DIRECTORY, 0, "pony");
    create_voidelle(vdom, pony->pos, 0, 0, "yeah");

    create_voidelle(vdom, vdom->root->pos, 0, 0, "lunar");
    create_voidelle(vdom, vdom->root->pos, 0, 0, "orbit");

    voidelle_t *forest = create_voidelle(vdom, vdom->root->pos, VOIDELLE_DIRECTORY, 0, "forest");
    voidelle_t *_27 = create_voidelle(vdom, vdom->root->pos, VOIDELLE_DIRECTORY, 0, "27");
    voidelle_t *promise = create_voidelle(vdom, vdom->root->pos, VOIDELLE_DIRECTORY, 0, "promise");
    create_voidelle(vdom, _27->pos, 0, 0, "honesty");
    create_voidelle(vdom, _27->pos, 0, 0, "twilight");
    create_voidelle(vdom, forest->pos, 0, 0, "chasing");
    create_voidelle(vdom, promise->pos, 0, 0, "hiii");
    voidelle_t *radiohead = create_voidelle(vdom, vdom->root->pos, 0, 0, "radiohead");
    printf("contest: %lu\n", radiohead->content);
    add_root(vdom, VOIDELLE, '>');

    vnode = fopen(">radiohead");
    fseek(vnode, 0, SEEK_SET);

    char buf[2096];
    unsigned long read = fread(buf, 2096, vnode);
    for (unsigned long i = 0; i < read; i++)
    {
        printf("%c", buf[i]);
    }

    printf("\n");

    while (1)
    {
        schedule();
    }
}