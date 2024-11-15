#ifndef WATCHDOG_H
#define WATCHDOG_H

void watchdog_start(unsigned int timeout);
void watchdog_stop();
unsigned int watchdog_remaining();

#endif