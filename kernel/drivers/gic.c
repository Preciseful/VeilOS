#include <drivers/gic.h>
#include <drivers/gpio.h>
#include <lib/printf.h>

void gic_allow(unsigned int interrupt, unsigned int core)
{
    unsigned int regi = interrupt / 32;
    unsigned int shift = interrupt % 32;
    unsigned long addr = GICD_ENABLE_IRQ_BASE + regi * 4;
    unsigned int val = mmio_read(addr);
    mmio_write(addr, val | (1 << shift));

    volatile unsigned char *target_byte = (volatile unsigned char *)(GIC_IRQ_TARGET_BASE + interrupt);
    *target_byte = (1 << core);
}