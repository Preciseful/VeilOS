#include <drivers/sdcard.h>
#include <lib/base.h>
#include <lib/printf.h>
#include <mailbox.h>
#include <drivers/timer.h>
#include <funcs.h>
#include <drivers/gpio.h>

// https://github.com/raspberrypi/linux/blob/rpi-6.6.y/drivers/mmc/host/bcm2835-sdhost.c

#define SD_BASE (PERIPHERAL_BASE + 0x00340000)
#define SD_CLOCK_FREQ 400000
#define SD_ARG2 (SD_BASE + 0)
#define SD_BLKSIZECNT (SD_BASE + 4)
#define SD_ARG1 (SD_BASE + 8)
#define SD_CMDTM (SD_BASE + 0xC)
#define SD_RESP0 (SD_BASE + 0x10)
#define SD_RESP1 (SD_BASE + 0x14)
#define SD_RESP2 (SD_BASE + 0x18)
#define SD_RESP3 (SD_BASE + 0x1C)
#define SD_DATA (SD_BASE + 0x20)
#define SD_STATUS (SD_BASE + 0x24)
#define SD_CONTROL0 (SD_BASE + 0x28)
#define SD_CONTROL1 (SD_BASE + 0x2C)
#define SD_INTERRUPT (SD_BASE + 0x30)
#define SD_IRPT_MASK (SD_BASE + 0x34)
#define SD_IRPT_EN (SD_BASE + 0x38)
#define SD_CONTROL2 (SD_BASE + 0x3C)
#define SD_CAPABILITIES_0 (SD_BASE + 0x40)
#define SD_CAPABILITIES_1 (SD_BASE + 0x44)
#define SD_FORCE_IRPT (SD_BASE + 0x50)
#define SD_BOOT_TIMEOUT (SD_BASE + 0x70)
#define SD_DBG_SEL (SD_BASE + 0x74)
#define SD_EXRDFIFO_CFG (SD_BASE + 0x80)
#define SD_EXRDFIFO_EN (SD_BASE + 0x84)
#define SD_TUNE_STEP (SD_BASE + 0x88)
#define SD_TUNE_STEPS_STD (SD_BASE + 0x8C)
#define SD_TUNE_STEPS_DDR (SD_BASE + 0x90)
#define SD_SPI_INT_SPT (SD_BASE + 0xF0)
#define SD_SLOTISR_VER (SD_BASE + 0xFC)

#define EMMC_HOST_RESET (1 << 24)
#define EMMC_ALL_RESET ((1 << 26) | (1 << 25) | (1 << 24))
#define EMMC_CLK_STABLE (1 << 1)
#define EMMC_CLK_INTERNAL_ENABLE (1 << 0)

enum EMMC_TAGS
{
    EMMC_GET_CLOCK_RATE = 0x30002
};

// taken from https://github.com/rockytriton/LLD/blob/57de54e48eb818d1feb421da5a88ce087fe79254/rpi_bm/part17/src/drivers/emmc/emmc_clock.c#L7
unsigned int calculate_clock_divider(unsigned int source_clock, unsigned int target_frequency)
{
    unsigned int divider = 1;

    // If the target frequency is less than or equal to the source clock
    if (target_frequency <= source_clock)
    {
        divider = source_clock / target_frequency;

        // If the division is not exact, set divider to 0
        if (source_clock % target_frequency != 0)
        {
            divider = 0;
        }
    }

    int divider_position = -1;

    // Loop through the bits to find the highest bit set in the divider
    for (int bit_position = 31; bit_position >= 0; bit_position--)
    {
        unsigned int bit_mask = (1 << bit_position);

        // Check if the current bit is set in the divider
        if (divider & bit_mask)
        {
            divider_position = bit_position; // Update the divider position
            divider &= ~bit_mask;            // Clear the bit in the divider

            // If there are any remaining bits in the divider, increment the position
            if (divider)
            {
                divider_position++;
            }

            break;
        }
    }

    // If no bit was found, set the divider position to 31
    if (divider_position == -1)
    {
        divider_position = 31;
    }

    // Ensure the divider position does not exceed 31
    if (divider_position >= 32)
    {
        divider_position = 31;
    }

    // If the position is not zero, convert it to a power of 2 (2^(divider_position - 1))
    if (divider_position != 0)
    {
        divider_position = (1 << (divider_position - 1));
    }

    // Cap the divider position at 0x3FF (1023)
    if (divider_position >= 0x400)
    {
        divider_position = 0x3FF;
    }

    // Encode the result into the appropriate register format
    unsigned int low_byte = divider_position & 0xFF;        // Lower 8 bits of the divider
    unsigned int high_byte = (divider_position >> 8) & 0x3; // Next 2 bits of the divider
    unsigned int encoded_result = (low_byte << 8) |         // Shift the low byte into place
                                  (high_byte << 6) |        // Shift the high byte into place
                                  (0 << 5);                 // Set the reserved 5th bit to 0

    return encoded_result;
}

unsigned int get_clock_rate()
{
    mailbox[0] = 7 * 4;
    mailbox[1] = MAILBOX_REQUEST;

    mailbox[2] = EMMC_GET_CLOCK_RATE;
    mailbox[3] = 4;
    mailbox[4] = 8;
    mailbox[5] = 1;
    mailbox[6] = 0;

    if (mailbox_call(GPU_TO_ARM))
    {
        return mailbox[6];
    }

    return 0;
}

void setup_clock()
{
    mmio_write(SD_CONTROL2, 0);
    unsigned int clock_rate = get_clock_rate();

    unsigned int ctrl = mmio_read(SD_CONTROL1);
    ctrl |= EMMC_CLK_INTERNAL_ENABLE;
    ctrl |= calculate_clock_divider(clock_rate, SD_CLOCK_FREQ);
    ctrl &= ~(0xf << 16);
    ctrl |= (11 << 16);

    mmio_write(SD_CONTROL1, ctrl);
    while (!(mmio_read(SD_CONTROL1) & EMMC_CLK_STABLE))
    {
        printf("\nWaiting for SD to stabilize.");
    }

    printf("\n");

    timer_sleep(3);

    ctrl = mmio_read(SD_CONTROL1);
    ctrl |= 4;
    mmio_write(SD_CONTROL1, ctrl);

    timer_sleep(3);
}

void sdcard_init()
{
    for (int i = 0; i <= 5; i++)
    {
        if (!gpio_function(34 + i, GPIO_FUNCTION_IN))
            printf("failed ig\n");

        gpio_setAlt3(48 + i);
        if (i == 0)
            gpio_pull(48, Pull_None);
        else
            gpio_pull(48 + i, Pull_Up);
    }

    // reset
    unsigned int control1 = mmio_read(SD_BASE + 0x2C);
    control1 |= (1 << 24);
    // Disable clock
    control1 &= ~(1 << 2);
    control1 &= ~(1 << 0);
    mmio_write(SD_CONTROL1, control1);

    while (mmio_read(SD_CONTROL1) & EMMC_ALL_RESET)
    {
        timer_sleep(1);
        printf("Waiting for SD_CONTROL1 to reset: %u", mmio_read(SD_CONTROL1));
    }

    while (!(mmio_read(SD_STATUS) & (1 << 16)))
    {
        printf("\nWaiting for valid card.");
    }

    setup_clock();
}