#include <drivers/rng.h>
#include <drivers/gpio.h>
#include <lib/printf.h>

#define RNG_BASE (PERIPHERAL_BASE + 0x104000)
#define RNG_CTRL (RNG_BASE + 0x00)
#define RNG_SOFT_RESET (RNG_BASE + 0x04)
#define RNG_FIFO_DATA (RNG_BASE + 0x20)
#define RNG_FIFO_COUNT (RNG_BASE + 0x24)
#define RNG_FIFO_MASK 0xFF

void RNGInit()
{
    WriteToMMIO(RNG_SOFT_RESET, 1);
    WriteToMMIO(RNG_SOFT_RESET, 0);

    unsigned int val = ReadMMIO(RNG_CTRL);
    val |= 1;
    WriteToMMIO(RNG_CTRL, val);
}

unsigned int GetRandom32()
{
    while (!(ReadMMIO(RNG_FIFO_COUNT) & RNG_FIFO_MASK))
        ;

    return ReadMMIO(RNG_FIFO_DATA);
}

unsigned long GetRandom64()
{
    return ((unsigned long)GetRandom32() << 32) | GetRandom32();
}