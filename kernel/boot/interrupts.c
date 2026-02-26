#include <boot/interrupts.h>
#include <boot/base.h>
#include <lib/printf.h>
#include <drivers/gpio.h>
#include <drivers/gic.h>
#include <drivers/timer.h>
#include <drivers/uart.h>
#include <scheduler/scheduler.h>
#include <scheduler/task.h>
#include <system/syscall.h>
#include <lib/string.h>

extern unsigned long dbg_regs[37];

unsigned long handle_vinvalid(unsigned long type, unsigned long esr, unsigned long elr, unsigned long far, InterruptStack *sp)
{
    unsigned int ec = esr >> 26;

    if (ec == 0b010101)
    {
        HandleSystemCall(sp);
        return 1;
    }

    LOG("\ninterrupt encountered:"
        "\n\ttype: %lu"
        "\n\tesr: %lu"
        "\n\telr: 0x%x"
        "\n\tfar: 0x%lx\n",
        type, esr, elr, far);
    return 0;
}

void handle_irq(InterruptStack registers[])
{
    unsigned int iar = ReadMMIO(GICC_IAR);
    unsigned int id = iar & 0x2FF;

    switch (id)
    {
    // generic timer
    case 30:
        // printf(".");
        refresh_cntp_tval(SYS_FREQ);
        WriteToMMIO(GICC_EOIR, iar);
        SchedulerTick(registers);
        break;

    // uart0
    case 153:
        UartReceived();
        WriteToMMIO(GICC_EOIR, iar);
        break;

    default:
        LOG("Unknown IRQ %u\n", id);
        WriteToMMIO(GICC_EOIR, iar);
        break;
    }
}