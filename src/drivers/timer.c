#include <drivers/timer.h>
#include <funcs.h>

const unsigned int timer1_int = CLOCKHZ;
const unsigned int timer3_int = CLOCKHZ;

unsigned int timer1_value = 0;
unsigned int timer3_value = 0;

unsigned int timer1_multiplier = 0;
unsigned int timer3_multiplier = 0;

timer_function timer1_function;
timer_function timer3_function;

void timer_init()
{
    timer1_value = TIMER->counter_low;
    timer1_value += timer1_int;
    TIMER->compare[1] = timer1_value;

    timer3_value = TIMER->counter_low;
    timer3_value += timer3_int;
    TIMER->compare[3] = timer3_value;
}

void set_timer_function(enum videocore_irqs irq, timer_function func)
{
    if (irq == SYS_TIMER_IRQ_1)
        timer1_function = func;
    else if (irq == SYS_TIMER_IRQ_3)
        timer3_function = func;
}

void remove_timer_function(enum videocore_irqs irq)
{
    if (irq == SYS_TIMER_IRQ_1)
        timer1_function = 0;
    else if (irq == SYS_TIMER_IRQ_3)
        timer3_function = 0;
}

unsigned long timer_get_ticks()
{
    unsigned int hi = TIMER->counter_high;
    unsigned int lo = TIMER->counter_low;

    // double check hi value didn't change after setting it...
    if (hi != TIMER->counter_high)
    {
        hi = TIMER->counter_high;
        lo = TIMER->counter_low;
    }

    return ((unsigned long)hi << 32) | lo;
}

void timer_sleep(unsigned int ms)
{
    unsigned int start = timer_get_ticks();

    while (timer_get_ticks() < start + (ms * 1000))
        ;
}

void handle_timer_1()
{
    timer1_value += timer1_int;
    TIMER->compare[1] = timer1_value;
    TIMER->control_status |= SYS_TIMER_IRQ_1;

    unsigned int counter = timer1_value / timer1_int;

    if (counter == __UINT32_MAX__)
        timer1_multiplier++;

    if (timer1_function != 0)
        timer1_function(counter, timer1_multiplier);
}

void handle_timer_3()
{
    timer3_value += timer3_int;
    TIMER->compare[3] = timer3_value;
    TIMER->control_status |= SYS_TIMER_IRQ_3;

    unsigned int counter = timer3_value / timer3_int;
    if (counter == __UINT32_MAX__)
        timer3_multiplier++;

    if (timer3_function != 0)
        timer3_function(counter, timer3_multiplier);
}