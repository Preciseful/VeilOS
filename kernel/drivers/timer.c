#include <drivers/timer.h>

void timer_sleep(unsigned int ms)
{
    unsigned long start = timer_ticks();

    while (timer_ticks() < start + (ms * 1000))
        ;
}