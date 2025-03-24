#ifndef FORK_H
#define FORK_H

#include <stdbool.h>
#include <scheduler.h>

#define PSR_MODE_EL0t 0x00000000
#define PSR_MODE_EL1t 0x00000004
#define PSR_MODE_EL1h 0x00000005
#define PSR_MODE_EL2t 0x00000008
#define PSR_MODE_EL2h 0x00000009
#define PSR_MODE_EL3t 0x0000000c
#define PSR_MODE_EL3h 0x0000000d

#ifdef __cplusplus
extern "C"
{
#endif

    struct pt_regs
    {
        unsigned long regs[31];
        unsigned long sp;
        unsigned long pc;
        unsigned long pstate;
    };

    bool fork(unsigned long fn, unsigned long arg, long kernel);
    extern void return_from_fork();

    int move_to_user_mode(unsigned long pc);
    struct pt_regs *task_pt_regs(struct task *tsk);

#ifdef __cplusplus
}
#endif
#endif