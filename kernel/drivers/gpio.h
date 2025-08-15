#pragma once

#include <boot/base.h>
#include <stdbool.h>

enum GPIO_Actions
{
    GPFSEL0 = PERIPHERAL_BASE + 0x200000,
    GPSET0 = PERIPHERAL_BASE + 0x20001C,
    GPCLR0 = PERIPHERAL_BASE + 0x200028,
    GPPUPPDN0 = PERIPHERAL_BASE + 0x2000E4
};

enum GPIO_Pulls
{
    Pull_None = 0,
    Pull_Down = 2,
    Pull_Up = 1
};

enum GPIO
{
    MAX_PIN = 53,
    GPIO_FUNCTION_IN = 0,
    GPIO_FUNCTION_OUT = 1,
    GPIO_FUNCTION_ALT5 = 2,
    GPIO_FUNCTION_ALT3 = 7,
    GPIO_FUNCTION_ALT0 = 4
};

void mmio_write(unsigned long reg, unsigned int val);
unsigned int mmio_read(unsigned long reg);

bool gpio_call(unsigned long pin, unsigned long value, enum GPIO_Actions base, unsigned long field_size, enum GPIO field_max);
bool gpio_set(unsigned long pin, unsigned long value);
bool gpio_clear(unsigned long pin, unsigned long value);
bool gpio_pull(unsigned long pin, enum GPIO_Pulls pull);
bool gpio_function(unsigned long pin, enum GPIO func);

void gpio_setAlt0(unsigned long pin);
void gpio_setAlt3(unsigned long pin);
void gpio_setAlt5(unsigned long pin);
void gpio_initOutputPinWithPullNone(unsigned long pin_number);
void gpio_setPinOutputBool(unsigned long pin, bool value);