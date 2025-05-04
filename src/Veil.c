#include <Veil.h>

#include <stdbool.h>
#include <stddef.h>

#include <funcs.h>
#include <interrupts.h>
#include <scheduler.h>
#include <mm.h>
#include <drivers/emmc.h>

#include <drivers/miniuart.h>
#include <drivers/watchdog.h>
#include <drivers/timer.h>
#include <drivers/framebuffer.h>
#include <drivers/emmc.h>

#include <lib/string.h>
#include <lib/printf.h>
#include <lib/fork.h>

#include <fs/vfs/vfs.h>
#include <modules.h>

unsigned char unveil_phrase[] = {'u', 'n', 'v', 'e', 'i', 'l'};

void kmain();

void unveil()
{
    uart_init();
    init_printf(0, putc);
    PSUCCESS("\n\n", "[INIT]\n");
    framebuffer_init();

    interrupt_init_vectors();
    timer_init();
    enable_vc_irq(SYS_TIMER_IRQ_1 | SYS_TIMER_IRQ_3);
    irq_barrier();
    irq_enable();
    breakpoint_enable();

    emmc_init();
    scheduler_init();

    vfs_init();

    modules_init();

    kmain();
}

void kmain()
{
    PSUCCESS("\n", "[KERNEL MAIN]\n");

    unsigned int el = get_el();
    INFO("with exception level %d\n", el);

    breakpoint();

    set_timer_function(SYS_TIMER_IRQ_1, scheduler_tick);
    int res;

    INFO("Framebuffer: %lu\n", (unsigned long)framebuffer);

    int *x = qalloc(int);
    *x = 100;
    printf("%d\n", *x);
    vfree(x);

    x = qalloc(int);
    *x = 500;
    printf("%d\n", *x);
    vfree(x);

    INFO("Scheduling is now taking place...\n");
    while (true)
        schedule();
}

void veil()
{
    emmc_command(CTGoIdle, 0, 2000);
    watchdog_start(0x120);
}