#include <boot/interrupts.h>
#include <boot/base.h>
#include <lib/printf.h>
#include <drivers/gpio.h>
#include <drivers/gic.h>
#include <drivers/timer.h>
#include <drivers/uart.h>
#include <scheduler/scheduler.h>
#include <scheduler/process.h>

void handle_svc(unsigned long *sp)
{
    unsigned long code = sp[8];
    switch (code)
    {
    case 0:
        printf("%c", sp[0]);
        break;

    case 1:
        sp[0] = VIRT_TO_PHYS((unsigned long)malloc(sp[0]));
        // this piece of code seems to make no sense as we pass a phys to virtual,
        // but it is because its virtual in user space
        map_task_page(get_running_task(), sp[0], MMU_RWRW, sp[0], PAGE_SIZE);
        break;

    case 2:
        if (!task_contains_va(get_running_task(), sp[0]))
            break;

        unsigned long len = free(PHYS_TO_VIRT((void *)sp[0]));
        unmap_task_page(get_running_task(), sp[0], len);
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
    unsigned int iar = mmio_read(GICC_IAR);
    unsigned int id = iar & 0x2FF;

    switch (id)
    {
    // generic timer
    case 30:
        // printf(".");
        refresh_cntp_tval(SYS_FREQ);
        mmio_write(GICC_EOIR, iar);
        scheduler_tick(stack);
        break;

    // uart0
    case 153:
        LOG("got: %c\n", uart_character());
        mmio_write(GICC_EOIR, iar);
        break;

    default:
        LOG("Unknown IRQ %u\n", id);
        mmio_write(GICC_EOIR, iar);
        break;
    }
}