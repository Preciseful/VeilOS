#ifndef SVC_H
#define SVC_H

static inline unsigned long svc(unsigned long svc_number)
{
    register unsigned long x8 asm("x8") = svc_number;
    register unsigned long x0 asm("x0") = 0;

    asm volatile(
        "svc #0"
        : "+r"(x0)
        : "r"(x8)
        : "memory", "cc");

    return x0;
}

static inline unsigned long svc0(unsigned long svc_number, unsigned long arg0)
{
    register unsigned long x8 asm("x8") = svc_number;
    register unsigned long x0 asm("x0") = arg0;

    asm volatile(
        "svc #0"
        : "+r"(x0)
        : "r"(x8)
        : "memory", "cc");

    return x0;
}

static inline unsigned long svc1(unsigned long svc_number, unsigned long arg0, unsigned long arg1)
{
    register unsigned long x8 asm("x8") = svc_number;
    register unsigned long x0 asm("x0") = arg0;
    register unsigned long x1 asm("x1") = arg1;

    asm volatile(
        "svc #0"
        : "+r"(x0)
        : "r"(x8), "r"(x1)
        : "memory", "cc");

    return x0;
}

static inline unsigned long svc2(unsigned long svc_number, unsigned long arg0, unsigned long arg1, unsigned long arg2)
{
    register unsigned long x8 asm("x8") = svc_number;
    register unsigned long x0 asm("x0") = arg0;
    register unsigned long x1 asm("x1") = arg1;
    register unsigned long x2 asm("x2") = arg2;

    asm volatile(
        "svc #0"
        : "+r"(x0)
        : "r"(x8), "r"(x1), "r"(x2)
        : "memory", "cc");

    return x0;
}

static inline unsigned long svc3(unsigned long svc_number, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
    register unsigned long x8 asm("x8") = svc_number;
    register unsigned long x0 asm("x0") = arg0;
    register unsigned long x1 asm("x1") = arg1;
    register unsigned long x2 asm("x2") = arg2;
    register unsigned long x3 asm("x3") = arg3;

    asm volatile(
        "svc #0"
        : "+r"(x0)
        : "r"(x8), "r"(x1), "r"(x2), "r"(x3)
        : "memory", "cc");

    return x0;
}

static inline unsigned long svc4(unsigned long svc_number, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3,
                                 unsigned long arg4)
{
    register unsigned long x8 asm("x8") = svc_number;
    register unsigned long x0 asm("x0") = arg0;
    register unsigned long x1 asm("x1") = arg1;
    register unsigned long x2 asm("x2") = arg2;
    register unsigned long x3 asm("x3") = arg3;
    register unsigned long x4 asm("x4") = arg4;

    asm volatile(
        "svc #0"
        : "+r"(x0)
        : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4)
        : "memory", "cc");

    return x0;
}

static inline unsigned long svc5(unsigned long svc_number, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3,
                                 unsigned long arg4, unsigned long arg5)
{
    register unsigned long x8 asm("x8") = svc_number;
    register unsigned long x0 asm("x0") = arg0;
    register unsigned long x1 asm("x1") = arg1;
    register unsigned long x2 asm("x2") = arg2;
    register unsigned long x3 asm("x3") = arg3;
    register unsigned long x4 asm("x4") = arg4;
    register unsigned long x5 asm("x5") = arg5;

    asm volatile(
        "svc #0"
        : "+r"(x0)
        : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5)
        : "memory", "cc");

    return x0;
}

#endif
