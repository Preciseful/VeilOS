#include <svc.h>

unsigned long svc_call(unsigned long x0_arg, unsigned long x1_arg, unsigned long x2_arg, unsigned long x3_arg,
                       unsigned long x4_arg, unsigned long x5_arg, unsigned long x6_arg, unsigned long x7_arg,
                       unsigned long svc_number)
{
  register unsigned long x0 asm("x0") = x0_arg;
  register unsigned long x1 asm("x1") = x1_arg;
  register unsigned long x2 asm("x2") = x2_arg;
  register unsigned long x3 asm("x3") = x3_arg;
  register unsigned long x4 asm("x4") = x4_arg;
  register unsigned long x5 asm("x5") = x5_arg;
  register unsigned long x6 asm("x6") = x6_arg;
  register unsigned long x7 asm("x7") = x7_arg;
  register unsigned long x8 asm("x8") = svc_number;

  asm volatile(
      "svc #0"
      : "+r"(x0)
      : "r"(x1), "r"(x2), "r"(x3),
        "r"(x4), "r"(x5), "r"(x6), "r"(x7),
        "r"(x8)
      : "memory", "cc");

  return x0;
}