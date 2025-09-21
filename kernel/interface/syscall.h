#pragma once

#define SYSCALL_HANDLER(name) unsigned long SystemCall_##name(unsigned long *sp)

void HandleSystemCall(unsigned long *sp);