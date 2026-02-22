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
#include <gdb/dbg.h>
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

#if RELEASE
    LOG("\ninterrupt encountered:"
        "\n\ttype: %lu"
        "\n\tesr: %lu"
        "\n\telr: 0x%x"
        "\n\tfar: 0x%lx",
        type, esr, elr, far);
    return 0;
#else
    memcpy(dbg_regs, sp, sizeof(unsigned long) * 28);
    dbg_regs[28] = sp->x28;
    dbg_regs[29] = sp->x29;
    dbg_regs[30] = sp->x30;
    dbg_regs[31] = sp->elr_el1;
    dbg_regs[32] = sp->spsr_el1;
    dbg_regs[33] = sp->esr_el1;
    dbg_regs[34] = sp->far_el1;
    dbg_regs[35] = sp->sctlr_el1;
    dbg_regs[36] = sp->tcr_el1;

    unsigned long excnum = 0;
    excnum = type % 4;
    dbg_main(excnum);

    memcpy(sp, dbg_regs, sizeof(unsigned long) * 28);
    sp->x28 = dbg_regs[28];
    sp->x29 = dbg_regs[29];
    sp->x30 = dbg_regs[30];
    sp->elr_el1 = dbg_regs[31];
    sp->spsr_el1 = dbg_regs[32];
    sp->esr_el1 = dbg_regs[33];
    sp->far_el1 = dbg_regs[34];
    sp->sctlr_el1 = dbg_regs[35];
    sp->tcr_el1 = dbg_regs[36];
    return 1;
#endif
}

void handle_irq(TaskRegs registers[])
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