#pragma once

#include <boot/base.h>
#include <stdbool.h>

enum GPIO_actions
{
    GPFSEL0 = PERIPHERAL_BASE + 0x200000,
    GPSET0 = PERIPHERAL_BASE + 0x20001C,
    GPCLR0 = PERIPHERAL_BASE + 0x200028,
    GPPUPPDN0 = PERIPHERAL_BASE + 0x2000E4
};

enum GPIO_pulls
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

void mmio_write(long reg, unsigned int val);
unsigned int mmio_read(long reg);

bool gpio_call(unsigned int pin, unsigned int value, enum GPIO_actions base, unsigned int field_size, enum GPIO field_max);
bool gpio_set(unsigned int pin, unsigned int value);
bool gpio_clear(unsigned int pin, unsigned int value);
bool gpio_pull(unsigned int pin, enum GPIO_pulls pull);
bool gpio_function(unsigned int pin, enum GPIO func);

void gpio_setAlt0(unsigned int pin);
void gpio_setAlt3(unsigned int pin);
void gpio_setAlt5(unsigned int pin);
void gpio_initOutputPinWithPullNone(unsigned int pin_number);
void gpio_setPinOutputBool(unsigned int pin, bool value);