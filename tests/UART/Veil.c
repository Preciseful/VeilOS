#include <Veil.h>

#include <stdbool.h>
#include <stddef.h>

#include <funcs.h>
#include <interrupts.h>
#include <scheduler.h>

#include <drivers/uart.h>
#include <drivers/watchdog.h>
#include <drivers/timer.h>

#include <lib/string.h>
#include <lib/printf.h>
#include <lib/fork.h>

unsigned char unveil_phrase[] = {'u', 'n', 'v', 'e', 'i', 'l'};

void kmain();

void unveil()
{
    uart_init();
    init_printf(0, putc);

    kmain();
}

void kmain()
{
    printf("abcdef");
    return;
}

void veil()
{
    watchdog_start(0x120);
}