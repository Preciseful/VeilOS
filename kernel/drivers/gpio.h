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

void WriteToMMIO(unsigned long reg, unsigned int val);
unsigned int ReadMMIO(unsigned long reg);

bool CallGPIO(unsigned long pin, unsigned long value, enum GPIO_Actions base, unsigned long field_size, enum GPIO field_max);
bool SetGPIO(unsigned long pin, unsigned long value);
bool ClearGPIO(unsigned long pin, unsigned long value);
bool PullGPIO(unsigned long pin, enum GPIO_Pulls pull);
bool SetGPIOFunction(unsigned long pin, enum GPIO func);

void SetAlt0(unsigned long pin);
void SetAlt3(unsigned long pin);
void SetAlt5(unsigned long pin);
void InitOutputPinWithPullNone(unsigned long pin_number);
void SetPinOutputBool(unsigned long pin, bool value);