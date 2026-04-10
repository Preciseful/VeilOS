#pragma once

#define SYS_FREQ 54000000

#ifndef __ASSEMBLER__

/**
 * @brief Initializes the timer.
 */
extern void timer_init();

/**
 * @brief Sets the `cntp_tval_el0` register to a value.
 * This sets the amount of time until the next interrupt that the timer should generate.
 * @param val The value.
 */
extern void refresh_cntp_tval(unsigned int val);

/**
 * @brief Get the amount of ticks that the timer has counted.
 *
 * @return The amount of ticks.
 */
extern unsigned long timer_ticks();

/**
 * @brief Wait until the timer reached current time + value in miliseconds.
 * @param ms The value in miliseconds.
 */
void TimerSleep(unsigned int ms);

#endif