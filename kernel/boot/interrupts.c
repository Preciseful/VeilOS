#include <boot/interrupts.h>
#include <boot/base.h>
#include <lib/printf.h>
#include <drivers/gpio.h>
#include <drivers/gic.h>
#include <drivers/timer.h>
#include <drivers/uart.h>
#include <scheduler/scheduler.h>
#include <scheduler/task.h>
#include <interface/portal.h>

void handle_portal(unsigned long *sp)
{
    unsigned long category = sp[0];
    unsigned long id = sp[1];

    Portal portal;
    if (!GetPortal(category, id, &portal))
        return;

    switch (sp[2])
    {
        // read
    case 0:
        if (portal.read)
            sp[0] = portal.read(portal.object, (unsigned char *)sp[3], sp[4]);
        else
            sp[0] = 0;
        break;

        // write
    case 1:
        if (portal.write)
            sp[0] = portal.write(portal.object, (unsigned char *)sp[3], sp[4]);
        else
            sp[0] = 0;
        break;

        // request
    case 2:
        if (portal.request)
            sp[0] = portal.request(portal.object, sp[3], (void *)sp[4]);
        else
            sp[0] = 0;
        break;

    default:
        break;
    }
}

void handle_svc(unsigned long *sp)
{
    unsigned long code = sp[8];
    switch (code)
    {
    case 0:
        handle_portal(sp);
        break;

    case 1:
        sp[0] = VIRT_TO_PHYS((unsigned long)malloc(sp[0]));
        // this piece of code seems to make no sense as we pass a phys to virtual,
        // but it is because its virtual in user space
        MapTaskPage(GetRunningTask(), sp[0], MMU_RWRW, sp[0], PAGE_SIZE);
        break;

    case 2:
        if (!TaskContainsVA(GetRunningTask(), sp[0]))
            break;

        unsigned long len = free(PHYS_TO_VIRT((void *)sp[0]));
        UnmapTaskPage(GetRunningTask(), sp[0], len);
        break;

    default:
        LOG("SVC code #%lu does not exist.\n", code);
        break;
    }
}

unsigned long handle_vinvalid(unsigned long type, unsigned long esr, unsigned long elr, unsigned long far, unsigned long *sp)
{
    unsigned int ec = esr >> 26;

    if (ec == 0b010101)
    {
        handle_svc(sp);
        return 1;
    }

    LOG("\ninterrupt encountered:"
        "\n\ttype: %lu"
        "\n\tesr: %lu"
        "\n\telr: 0x%lx"
        "\n\tfar: 0x%lx",
        type, esr, elr, far);
    return 0;
}

void handle_irq(unsigned long *stack)
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
        SchedulerTick(stack);
        break;

    // uart0
    case 153:
        LOG("got: %c\n", UartCharacter());
        WriteToMMIO(GICC_EOIR, iar);
        break;

    default:
        LOG("Unknown IRQ %u\n", id);
        WriteToMMIO(GICC_EOIR, iar);
        break;
    }
}