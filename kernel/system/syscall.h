#pragma once

#include <boot/interrupts.h>

#define SYSCALL_HANDLER(name) unsigned long SystemCall_##name(InterruptStack *sp)

/**
 * @brief Handles a system call, by checking the ID and priority.
 *
 * @param sp The interrupt stack.
 */
void HandleSystemCall(InterruptStack sp[]);