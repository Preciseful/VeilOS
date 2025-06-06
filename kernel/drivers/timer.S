#include "drivers/timer.h"

.global refresh_cntp_tval
refresh_cntp_tval:
    msr cntp_tval_el0, x0
    ret

.global timer_ticks
timer_ticks:
    mrs x0, cntpct_el0
    ret

.global timer_init
timer_init:
    movz x0, #(SYS_FREQ & 0xFFFF), lsl #0
    movk x0, #((SYS_FREQ >> 16) & 0xFFFF), lsl #16
    msr cntp_tval_el0, x0
    mov x9, 1
    msr cntp_ctl_el0, x9
    ret