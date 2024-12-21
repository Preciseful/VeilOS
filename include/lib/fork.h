#ifndef FORK_H
#define FORK_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    bool fork(unsigned long fn, unsigned long arg);
    extern void return_from_fork();

#ifdef __cplusplus
}
#endif
#endif