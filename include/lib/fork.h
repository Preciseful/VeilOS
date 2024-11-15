#ifndef FORK_H
#define FORK_H

#include <stdbool.h>

bool fork(unsigned long fn, unsigned long arg);
extern void return_from_fork();

#endif