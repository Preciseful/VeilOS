#include <drivers/gpio.h>

void WriteToMMIO(unsigned long reg, unsigned int val)
{
    *((volatile unsigned int *)reg) = val;
}

unsigned int ReadMMIO(unsigned long reg)
{
    return *((volatile unsigned int *)reg);
}

bool CallGPIO(unsigned long pin, unsigned long value, enum GPIO_Actions base, unsigned long field_size, enum GPIO field_max)
{
    unsigned long field_mask = (1 << field_size) - 1;

    if (pin > field_mask && value > field_mask)
        return false;

    unsigned long num_fields = 32 / field_size;
    unsigned long regist = base + ((pin / num_fields) * 4);
    unsigned long shift = (pin % num_fields) * field_size;

    unsigned long finalval = ReadMMIO(regist);
    finalval &= ~(field_mask << shift);
    finalval |= value << shift;

    WriteToMMIO(regist, finalval);

    return true;
}

bool SetGPIO(unsigned long pin, unsigned long value)
{
    return CallGPIO(pin, value, GPSET0, 1, MAX_PIN);
}

bool ClearGPIO(unsigned long pin, unsigned long value)
{
    return CallGPIO(pin, value, GPCLR0, 1, MAX_PIN);
}

bool PullGPIO(unsigned long pin, enum GPIO_Pulls pull)
{
    return CallGPIO(pin, pull, GPPUPPDN0, 2, MAX_PIN);
}

bool SetGPIOFunction(unsigned long pin, enum GPIO func)
{
    return CallGPIO(pin, func, GPFSEL0, 3, MAX_PIN);
}

void SetAlt0(unsigned long pin)
{
    PullGPIO(pin, Pull_None);
    SetGPIOFunction(pin, GPIO_FUNCTION_ALT0);
}

void SetAlt3(unsigned long pin)
{
    PullGPIO(pin, Pull_None);
    SetGPIOFunction(pin, GPIO_FUNCTION_ALT3);
}

void SetAlt5(unsigned long pin)
{
    PullGPIO(pin, Pull_None);
    SetGPIOFunction(pin, GPIO_FUNCTION_ALT5);
}

void InitOutputPinWithPullNone(unsigned long pin_number)
{
    PullGPIO(pin_number, Pull_None);
    SetGPIOFunction(pin_number, GPIO_FUNCTION_OUT);
}

void SetPinOutputBool(unsigned long pin, bool value)
{
    if (value)
        SetGPIO(pin, 1);
    else
        ClearGPIO(pin, 1);
}