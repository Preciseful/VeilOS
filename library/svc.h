#pragma once

enum System_Calls
{
    SYS_PORTAL,
    SYS_MALLOC,
    SYS_FREE,
    SYS_GET_MEMORY_SIZE,
    SYS_EXIT_PROCESS,
    SYS_SET_ENVIRON,
    SYS_EXECVE
};

unsigned long svc_call(unsigned long x0_arg, unsigned long x1_arg, unsigned long x2_arg, unsigned long x3_arg,
                       unsigned long x4_arg, unsigned long x5_arg, unsigned long x6_arg, unsigned long x7_arg,
                       unsigned long svc_number);