#include <drivers/gpio.h>

void WriteToMMIO(VirtualAddr reg, unsigned int val)
{
    *((volatile unsigned int *)reg) = val;
}

unsigned int ReadMMIO(VirtualAddr reg)
{
    return *((volatile unsigned int *)reg);
}

bool CallGPIO(unsigned long pin, unsigned long value, enum GPIO_Actions base, unsigned long field_size)
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
    return CallGPIO(pin, value, GPIOSET0, 1);
}

bool ClearGPIO(unsigned long pin, unsigned long value)
{
    return CallGPIO(pin, value, GPIOCLR0, 1);
}

bool PullGPIO(unsigned long pin, enum GPIO_Pulls pull)
{
    return CallGPIO(pin, pull, GPIOPUPPDN0, 2);
}

bool SetGPIOFunction(unsigned long pin, enum GPIO func)
{
    return CallGPIO(pin, func, GPIOFSEL0, 3);
}

void SetAlt0(unsigned long pin)
{
    PullGPIO(pin, GPIO_PULL_NONE);
    SetGPIOFunction(pin, GPIO_FUNCTION_ALT0);
}

void SetAlt3(unsigned long pin)
{
    PullGPIO(pin, GPIO_PULL_NONE);
    SetGPIOFunction(pin, GPIO_FUNCTION_ALT3);
}

void SetAlt5(unsigned long pin)
{
    PullGPIO(pin, GPIO_PULL_NONE);
    SetGPIOFunction(pin, GPIO_FUNCTION_ALT5);
}

void SetPinOutputBool(unsigned long pin, bool value)
{
    if (value)
        SetGPIO(pin, 1);
    else
        ClearGPIO(pin, 1);
}