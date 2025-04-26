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
    printf("\n\n[INIT]\n");
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

void input_process(unsigned long args)
{
    int i = 0;
    unsigned char read[6] = {'\0'};

    while (true)
    {
        preempt_disable();

        unsigned char current = uart_update();
        if ((unsigned int)current == 0)
            continue;

        read[i] = current;

        if (read[i] == unveil_phrase[i])
            i++;
        else
            i = 0;

        if (i == 6)
            veil();

        preempt_enable();
        scheduler_move_next();
    }
}

void stage(unsigned long args)
{
    int i = 0;
    while (true)
    {
        i++;
        printf("b");
        delay(10000);
    }
}

void process(unsigned long args)
{
    int i = 0;
    while (true)
    {
        i++;
        printf("a");
        delay(10000);
    }
}

void kernel_process(unsigned long args)
{
    printf("Kernel process started. EL %d\r\n", get_el());
    set_pc((unsigned long)&process);
}

void kmain()
{
    printf("\n[KERNEL MAIN]\n");

    unsigned int el = get_el();
    printf("with exception level %d\n", el);

    breakpoint();

    set_timer_function(SYS_TIMER_IRQ_1, scheduler_tick);
    int res;

    res = fork((unsigned long)&kernel_process, "A", 1);
    printf("Fork result: %d\n", res);

    res = fork((unsigned long)&stage, "B", 1);
    printf("Fork result: %d\n", res);

    printf("Framebuffer: %lu\n", (unsigned long)framebuffer);

    int *x = qalloc(int);
    *x = 100;
    printf("%d\n", *x);
    vfree(x);

    x = qalloc(int);
    *x = 500;
    printf("%d\n", *x);
    vfree(x);

    printf("Scheduling is now taking place...\n");
    while (true)
        schedule();
}

void veil()
{
    emmc_command(CTGoIdle, 0, 2000);
    watchdog_start(0x120);
}