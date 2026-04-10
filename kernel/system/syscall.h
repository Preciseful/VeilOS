/**
 * @file
 * @author Developful
 * @brief System calls for ARM interface.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <boot/interrupts.h>

#define SYSCALL_HANDLER(name) unsigned long SystemCall_##name(InterruptStack *sp)

/**
 * @brief Handles a system call, by checking the ID and priority.
 *
 * @param sp The interrupt stack.
 */
void HandleSystemCall(InterruptStack sp[]);