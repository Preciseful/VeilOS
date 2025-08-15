#include <drivers/gpio.h>

void mmio_write(unsigned long reg, unsigned int val)
{
    *((volatile unsigned int *)reg) = val;
}

unsigned int mmio_read(unsigned long reg)
{
    return *((volatile unsigned int *)reg);
}

bool gpio_call(unsigned long pin, unsigned long value, enum GPIO_Actions base, unsigned long field_size, enum GPIO field_max)
{
    unsigned long field_mask = (1 << field_size) - 1;

    if (pin > field_mask && value > field_mask)
        return false;

    unsigned long num_fields = 32 / field_size;
    unsigned long regist = base + ((pin / num_fields) * 4);
    unsigned long shift = (pin % num_fields) * field_size;

    unsigned long finalval = mmio_read(regist);
    finalval &= ~(field_mask << shift);
    finalval |= value << shift;

    mmio_write(regist, finalval);

    return true;
}

bool gpio_set(unsigned long pin, unsigned long value)
{
    return gpio_call(pin, value, GPSET0, 1, MAX_PIN);
}

bool gpio_clear(unsigned long pin, unsigned long value)
{
    return gpio_call(pin, value, GPCLR0, 1, MAX_PIN);
}

bool gpio_pull(unsigned long pin, enum GPIO_Pulls pull)
{
    return gpio_call(pin, pull, GPPUPPDN0, 2, MAX_PIN);
}

bool gpio_function(unsigned long pin, enum GPIO func)
{
    return gpio_call(pin, func, GPFSEL0, 3, MAX_PIN);
}

void gpio_setAlt0(unsigned long pin)
{
    gpio_pull(pin, Pull_None);
    gpio_function(pin, GPIO_FUNCTION_ALT0);
}

void gpio_setAlt3(unsigned long pin)
{
    gpio_pull(pin, Pull_None);
    gpio_function(pin, GPIO_FUNCTION_ALT3);
}

void gpio_setAlt5(unsigned long pin)
{
    gpio_pull(pin, Pull_None);
    gpio_function(pin, GPIO_FUNCTION_ALT5);
}

void gpio_initOutputPinWithPullNone(unsigned long pin_number)
{
    gpio_pull(pin_number, Pull_None);
    gpio_function(pin_number, GPIO_FUNCTION_OUT);
}

void gpio_setPinOutputBool(unsigned long pin, bool value)
{
    if (value)
        gpio_set(pin, 1);
    else
        gpio_clear(pin, 1);
}