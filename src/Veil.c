#include <Veil.h>

#include <stdbool.h>
#include <stddef.h>

#include <funcs.h>
#include <interrupts.h>
#include <scheduler.h>

#include <drivers/miniuart.h>
#include <drivers/watchdog.h>
#include <drivers/timer.h>
#include <drivers/framebuffer.h>

#include <lib/string.h>
#include <lib/printf.h>
#include <lib/fork.h>

unsigned char unveil_phrase[] = {'u', 'n', 'v', 'e', 'i', 'l'};

void kmain();

void unveil()
{
    uart_init();
    init_printf(0, putc);
    framebuffer_init();
    interrupt_init_vectors();
    timer_init();
    scheduler_init();

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

void process(unsigned long args)
{
    int i = 0;
    while (true)
    {
        i++;
        delay(10000);
    }
}

void kmain()
{
    // drawString(0, 2, "eat this up like candy", 0x0f, 2);
    printf("\n\nhello world!!\n");

    unsigned int el = get_el();
    printf("with exception level %d\n", el);

    enable_vc_irq(SYS_TIMER_IRQ_1 | SYS_TIMER_IRQ_3);
    irq_barrier();
    irq_enable();

    breakpoint_enable();
    breakpoint();

    set_timer_function(SYS_TIMER_IRQ_1, scheduler_tick);

    int res = fork((unsigned long)&input_process, 0);
    printf("%d\n", res);

    res = fork((unsigned long)&process, 0);
    printf("%d\n", res);

    printf("%lu\n", (unsigned long)framebuffer);

    while (true)
        schedule();
}

void veil()
{
    watchdog_start(0x120);
}