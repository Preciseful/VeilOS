#pragma once

#include <boot/interrupts.h>

#define SYSCALL_HANDLER(name) unsigned long SystemCall_##name(InterruptStack *sp)

void HandleSystemCall(InterruptStack *sp);