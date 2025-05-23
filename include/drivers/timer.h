#ifndef TIMER_H
#define TIMER_H

#include <lib/base.h>
#include <interrupts.h>

// clang-format off

#ifdef __cplusplus
extern "C"
{
#endif

#define TIMER ((struct timer_registers *)(PERIPHERAL_BASE + 0x00003000))
#define CLOCKHZ 500000
typedef void (*timer_function)(unsigned int counter, unsigned int multiplier);

struct timer_registers
{
    volatile unsigned int control_status;
    volatile unsigned int counter_low;
    volatile unsigned int counter_high;
    volatile unsigned int compare[4];
};

void timer_init();
void set_timer_function(enum videocore_irqs irq, timer_function func);
void remove_timer_function(enum videocore_irqs irq);
void timer_sleep(unsigned int ms);
unsigned long timer_get_ticks();

void handle_timer_1();
void handle_timer_3();

#ifdef __cplusplus
}
#endif

#endif