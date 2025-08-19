#include <drivers/timer.h>

void TimerSleep(unsigned int ms)
{
    unsigned long start = timer_ticks();

    while (timer_ticks() < start + (ms * 1000))
        ;
}