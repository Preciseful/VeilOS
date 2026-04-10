/**
 * @author Developful
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <drivers/timer.h>

void TimerSleep(unsigned int ms)
{
    unsigned long start = timer_ticks();

    while (timer_ticks() < start + (ms * 1000))
        ;
}