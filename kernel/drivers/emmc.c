#include <drivers/mailbox.h>
#include <drivers/emmc.h>
#include <drivers/gpio.h>
#include <drivers/timer.h>
#include <lib/printf.h>

// HUGE THANKS to https://github.com/rockytriton/LLD/blob/main/rpi_bm/part17

static emmc_device device = {0};
static const emmc_cmd INVALID_CMD = RES_CMD;

bool wait_reg_mask(volatile unsigned int *reg, unsigned int mask, bool set, unsigned int timeout)
{
    for (int cycles = 0; cycles <= timeout; cycles++)
    {
        if ((*reg & mask) ? set : !set)
        {
            return true;
        }

        timer_sleep(1);
    }

    return false;
}

unsigned int get_clock_divider(unsigned int base_clock, unsigned int target_rate)
{
    unsigned int target_div = 1;

    if (target_rate <= base_clock)
    {
        target_div = base_clock / target_rate;

        if (base_clock % target_rate)
        {
            target_div = 0;
        }
    }

    int div = -1;
    for (int fb = 31; fb >= 0; fb--)
    {
        unsigned int bt = (1 << fb);

        if (target_div & bt)
        {
            div = fb;
            target_div &= ~(bt);

            if (target_div)
            {
                div++;
            }

            break;
        }
    }

    if (div == -1)
    {
        div = 31;
    }

    if (div >= 32)
    {
        div = 31;
    }

    if (div != 0)
    {
        div = (1 << (div - 1));
    }

    if (div >= 0x400)
    {
        div = 0x3FF;
    }

    unsigned int freqSel = div & 0xff;
    unsigned int upper = (div >> 8) & 0x3;
    unsigned int ret = (freqSel << 8) | (upper << 6) | (0 << 5);

    return ret;
}

bool switch_clock_rate(unsigned int base_clock, unsigned int target_rate)
{
    unsigned int divider = get_clock_divider(base_clock, target_rate);

    while ((EMMC->status & (EMMC_STATUS_CMD_INHIBIT | EMMC_STATUS_DAT_INHIBIT)))
    {
        timer_sleep(1);
    }

    unsigned int c1 = EMMC->control[1] & ~EMMC_CTRL1_CLK_ENABLE;

    EMMC->control[1] = c1;

    timer_sleep(3);

    EMMC->control[1] = (c1 | divider) & ~0xFFE0;

    timer_sleep(3);

    EMMC->control[1] = c1 | EMMC_CTRL1_CLK_ENABLE;

    timer_sleep(3);

    return true;
}

bool emmc_setup_clock()
{
    EMMC->control2 = 0;

    unsigned int rate = mailbox_clock(EMMC_CLOCK);

    unsigned int n = EMMC->control[1];
    n |= EMMC_CTRL1_CLK_INT_EN;
    n |= get_clock_divider(rate, SD_CLOCK_ID);
    n &= ~(0xf << 16);
    n |= (11 << 16);

    EMMC->control[1] = n;

    if (!wait_reg_mask(&EMMC->control[1], EMMC_CTRL1_CLK_STABLE, true, 2000))
    {
        printf("SD clock not stable.\n");
        return false;
    }

    timer_sleep(30);

    // enabling the clock
    EMMC->control[1] |= 4;

    timer_sleep(30);

    return true;
}

static const emmc_cmd commands[] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    RES_CMD,
    {0, 0, 0, 0, 0, 0, RT136, 0, 1, 0, 0, 0, 2, 0},
    {0, 0, 0, 0, 0, 0, RT48, 0, 1, 0, 0, 0, 3, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0},
    {0, 0, 0, 0, 0, 0, RT136, 0, 0, 0, 0, 0, 5, 0},
    {0, 0, 0, 0, 0, 0, RT48, 0, 1, 0, 0, 0, 6, 0},
    {0, 0, 0, 0, 0, 0, RT48Busy, 0, 1, 0, 0, 0, 7, 0},
    {0, 0, 0, 0, 0, 0, RT48, 0, 1, 0, 0, 0, 8, 0},
    {0, 0, 0, 0, 0, 0, RT136, 0, 1, 0, 0, 0, 9, 0},
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    {0, 0, 0, 0, 0, 0, RT48, 0, 1, 0, 0, 0, 16, 0},
    {0, 0, 0, 1, 0, 0, RT48, 0, 1, 0, 1, 0, 17, 0},
    {0, 1, 1, 1, 1, 0, RT48, 0, 1, 0, 1, 0, 18, 0},
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    {0, 0, 0, 0, 0, 0, RT48Busy, 0, 1, 0, 1, 0, 24, 0},
    {0, 1, 1, 0, 1, 0, RT48Busy, 0, 1, 0, 1, 0, 25, 0},
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    {0, 0, 0, 0, 0, 0, RT48, 0, 0, 0, 0, 0, 41, 0},
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    RES_CMD,
    {0, 0, 0, 1, 0, 0, RT48, 0, 1, 0, 1, 0, 51, 0},
    RES_CMD,
    RES_CMD,
    RES_CMD,
    {0, 0, 0, 0, 0, 0, RT48, 0, 1, 0, 0, 0, 55, 0},
};

static unsigned int sd_error_mask(sd_error err)
{
    return 1 << (16 + (unsigned int)err);
}

static void set_last_error(unsigned int intr_val)
{
    device.last_error = intr_val & 0xFFFF0000;
    device.last_interrupt = intr_val;
}

static bool do_data_transfer(emmc_cmd cmd)
{
    unsigned int wrIrpt = 0;
    bool write = false;

    if (cmd.direction)
    {
        wrIrpt = 1 << 5;
    }
    else
    {
        wrIrpt = 1 << 4;
        write = true;
    }

    unsigned int *data = (unsigned int *)device.buffer;

    for (int block = 0; block < device.transfer_blocks; block++)
    {
        wait_reg_mask(&EMMC->int_flags, wrIrpt | 0x8000, true, 2000);
        unsigned int intr_val = EMMC->int_flags;
        EMMC->int_flags = wrIrpt | 0x8000;

        if ((intr_val & (0xffff0000 | wrIrpt)) != wrIrpt)
        {
            set_last_error(intr_val);
            return false;
        }

        unsigned int length = device.block_size;

        if (write)
        {
            for (; length > 0; length -= 4)
            {
                EMMC->data = *data++;
            }
        }
        else
        {
            for (; length > 0; length -= 4)
            {
                *data++ = EMMC->data;
            }
        }
    }

    return true;
}

static bool emmc_issue_command(emmc_cmd cmd, unsigned int arg, unsigned int timeout)
{
    device.last_command_value = TO_REG(&cmd);
    volatile unsigned int command_reg = device.last_command_value;

    if (device.transfer_blocks > 0xFFFF)
    {
        printf("Transfer blocks too large: %d\n", device.transfer_blocks);
        return false;
    }

    EMMC->block_size_count = device.block_size | (device.transfer_blocks << 16);
    EMMC->arg1 = arg;
    EMMC->cmd_xfer_mode = command_reg;

    timer_sleep(10);

    int times = 0;

    while (times < timeout)
    {
        unsigned int reg = EMMC->int_flags;

        if (reg & 0x8001)
        {
            break;
        }

        timer_sleep(1);
        times++;
    }

    if (times >= timeout)
    {
        // just doing a warn for this because sometimes it's ok.
        printf("Emmc_issue_command timed out\n");
        device.last_success = false;
        return false;
    }

    unsigned int intr_val = EMMC->int_flags;

    EMMC->int_flags = 0xFFFF0001;

    if ((intr_val & 0xFFFF0001) != 1)
    {

        if (EMMC_DEBUG)
            printf("Error waiting for command interrupt complete: %d\n", cmd.index);

        set_last_error(intr_val);

        if (EMMC_DEBUG)
            printf("IRQFLAGS: %X - %X - %X\n", EMMC->int_flags, EMMC->status, intr_val);

        device.last_success = false;
        return false;
    }

    switch (cmd.response_type)
    {
    case RT48:
    case RT48Busy:
        device.last_response[0] = EMMC->response[0];
        break;

    case RT136:
        device.last_response[0] = EMMC->response[0];
        device.last_response[1] = EMMC->response[1];
        device.last_response[2] = EMMC->response[2];
        device.last_response[3] = EMMC->response[3];
        break;
    }

    if (cmd.is_data)
    {
        do_data_transfer(cmd);
    }

    if (cmd.response_type == RT48Busy || cmd.is_data)
    {
        wait_reg_mask(&EMMC->int_flags, 0x8002, true, 2000);
        intr_val = EMMC->int_flags;

        EMMC->int_flags = 0xFFFF0002;

        if ((intr_val & 0xFFFF0002) != 2 && (intr_val & 0xFFFF0002) != 0x100002)
        {
            set_last_error(intr_val);
            return false;
        }

        EMMC->int_flags = 0xFFFF0002;
    }

    device.last_success = true;

    return true;
}

bool emmc_command(unsigned int command, unsigned int arg, unsigned int timeout)
{
    if (command & 0x80000000)
    {
        // The app command flag is set, shoudl use emmc_app_command instead.
        printf("Use emmc_app_command instead for %x\n", command);
        return false;
    }

    device.last_command = commands[command];

    if (TO_REG(&device.last_command) == TO_REG(&INVALID_CMD))
    {
        printf("Command's reserved.\n");
        return false;
    }

    return emmc_issue_command(device.last_command, arg, timeout);
}

static bool reset_command()
{
    EMMC->control[1] |= EMMC_CTRL1_RESET_CMD;

    for (int i = 0; i < 10000; i++)
    {
        if (!(EMMC->control[1] & EMMC_CTRL1_RESET_CMD))
        {
            return true;
        }

        timer_sleep(1);
    }

    printf("Command line failed to reset properly: %X\n", EMMC->control[1]);

    return false;
}

bool emmc_app_command(unsigned int command, unsigned int arg, unsigned int timeout)
{

    if (commands[command].index >= 60)
    {
        printf("Invalid app command.\n");
        return false;
    }

    device.last_command = commands[CTApp];

    unsigned int rca = 0;

    if (device.rca)
    {
        rca = device.rca << 16;
    }

    if (emmc_issue_command(device.last_command, rca, 2000))
    {
        device.last_command = commands[command];

        return emmc_issue_command(device.last_command, arg, 2000);
    }

    return false;
}

static bool check_v2_card()
{
    bool v2Card = false;

    if (!emmc_command(CTSendIfCond, 0x1AA, 200))
    {
        if (device.last_error == 0)
        {
            // timeout.
            printf("SEND_IF_COND Timeout\n");
        }
        else if (device.last_error & (1 << 16))
        {
            // timeout command error
            if (!reset_command())
            {
                return false;
            }

            EMMC->int_flags = sd_error_mask(SDECommandTimeout);
            printf("SEND_IF_COND CMD TIMEOUT\n");
        }
        else
        {
            printf("Failure sending SEND_IF_COND\n");
            return false;
        }
    }
    else
    {
        if ((device.last_response[0] & 0xFFF) != 0x1AA)
        {
            printf("Unusable SD Card: %X\n", device.last_response[0]);
            return false;
        }

        v2Card = true;
    }

    return v2Card;
}

static bool check_usable_card()
{
    if (!emmc_command(CTIOSetOpCond, 0, 1000))
    {
        if (device.last_error == 0)
        {
            // timeout.
            printf("CTIOSetOpCond Timeout\n");
        }
        else if (device.last_error & (1 << 16))
        {
            // timeout command error
            // this is a normal expected error and calling the reset command will fix it.
            if (!reset_command())
            {
                return false;
            }

            EMMC->int_flags = sd_error_mask(SDECommandTimeout);
        }
        else
        {
            printf("SDIO Card not supported\n");
            return false;
        }
    }

    return true;
}

static bool check_sdhc_support(bool v2_card)
{
    bool card_busy = true;

    while (card_busy)
    {
        unsigned int v2_flags = 0;

        if (v2_card)
        {
            v2_flags |= (1 << 30); // SDHC Support
        }

        if (!emmc_app_command(CTOcrCheck, 0x00FF8000 | v2_flags, 2000))
        {
            printf("APP CMD 41 FAILED 2nd\n");
            return false;
        }

        if (device.last_response[0] >> 31 & 1)
        {
            device.ocr = (device.last_response[0] >> 8 & 0xFFFF);
            device.sdhc = ((device.last_response[0] >> 30) & 1) != 0;
            card_busy = false;
        }
        else
        {
            if (EMMC_DEBUG)
                printf("Sleeping: %X\n", device.last_response[0]);
            timer_sleep(500);
        }
    }

    return true;
}

static bool check_ocr()
{
    bool passed = false;

    for (int i = 0; i < 5; i++)
    {
        if (!emmc_app_command(CTOcrCheck, 0, 2000))
        {
            printf("APP CMD OCR CHECK TRY %d FAILED\n", i + 1);
            passed = false;
        }
        else
        {
            passed = true;
        }

        if (passed)
        {
            break;
        }

        return false;
    }

    if (!passed)
    {
        printf("APP CMD 41 FAILED\n");
        return false;
    }

    device.ocr = (device.last_response[0] >> 8 & 0xFFFF);

    if (EMMC_DEBUG)
        printf("MEMORY OCR: %X\n", device.ocr);

    return true;
}

static bool check_rca()
{
    if (!emmc_command(CTSendCide, 0, 2000))
    {
        printf("Failed to send CID\n");

        return false;
    }

    if (EMMC_DEBUG)
        printf("CARD ID: %X.%X.%X.%X\n", device.last_response[0], device.last_response[1], device.last_response[2], device.last_response[3]);

    if (!emmc_command(CTSendRelativeAddr, 0, 2000))
    {
        printf("Failed to send Relative Addr\n");

        return false;
    }

    device.rca = (device.last_response[0] >> 16) & 0xFFFF;

    if (EMMC_DEBUG)
    {
        printf("RCA: %X\n", device.rca);

        printf("CRC_ERR: %d\n", (device.last_response[0] >> 15) & 1);
        printf("CMD_ERR: %d\n", (device.last_response[0] >> 14) & 1);
        printf("GEN_ERR: %d\n", (device.last_response[0] >> 13) & 1);
        printf("STS_ERR: %d\n", (device.last_response[0] >> 9) & 1);
        printf("READY  : %d\n", (device.last_response[0] >> 8) & 1);
    }

    if (!((device.last_response[0] >> 8) & 1))
    {
        printf("Failed to read RCA\n");
        return false;
    }

    return true;
}

static bool select_card()
{
    if (!emmc_command(CTSelectCard, device.rca << 16, 2000))
    {
        printf("Failed to select card\n");
        return false;
    }

    if (EMMC_DEBUG)
        printf("Selected Card\n");

    unsigned int status = (device.last_response[0] >> 9) & 0xF;

    if (status != 3 && status != 4)
    {
        printf("Invalid Status: %d\n", status);
        return false;
    }

    if (EMMC_DEBUG)
        printf("Status: %d\n", status);

    return true;
}

static bool set_scr()
{
    if (!device.sdhc)
    {
        if (!emmc_command(CTSetBlockLen, 512, 2000))
        {
            printf("Failed to set block len\n");
            return false;
        }
    }

    unsigned int bsc = EMMC->block_size_count;
    bsc &= ~0xFFF; // mask off bottom bits
    bsc |= 0x200;  // set bottom bits to 512
    EMMC->block_size_count = bsc;

    device.buffer = &device.scr.scr[0];
    device.block_size = 8;
    device.transfer_blocks = 1;

    if (!emmc_app_command(CTSendSCR, 0, 30000))
    {
        printf("Failed to send SCR\n");
        return false;
    }

    if (EMMC_DEBUG)
        printf("GOT SRC: SCR0: %X SCR1: %X BWID: %X\n", device.scr.scr[0], device.scr.scr[1], device.scr.bus_widths);

    device.block_size = 512;

    unsigned int scr0 = BSWAP32(device.scr.scr[0]);
    device.scr.version = 0xFFFFFFFF;
    unsigned int spec = (scr0 >> (56 - 32)) & 0xf;
    unsigned int spec3 = (scr0 >> (47 - 32)) & 0x1;
    unsigned int spec4 = (scr0 >> (42 - 32)) & 0x1;

    if (spec == 0)
    {
        device.scr.version = 1;
    }
    else if (spec == 1)
    {
        device.scr.version = 11;
    }
    else if (spec == 2)
    {

        if (spec3 == 0)
        {
            device.scr.version = 2;
        }
        else if (spec3 == 1)
        {
            if (spec4 == 0)
            {
                device.scr.version = 3;
            }
            if (spec4 == 1)
            {
                device.scr.version = 4;
            }
        }
    }

    if (EMMC_DEBUG)
        printf("SCR Version: %d\n", device.scr.version);

    return true;
}

static bool emmc_card_reset()
{
    EMMC->control[1] = EMMC_CTRL1_RESET_HOST;

    if (EMMC_DEBUG)
        printf("Card resetting...\n");

    if (!wait_reg_mask(&EMMC->control[1], EMMC_CTRL1_RESET_ALL, false, 2000))
    {
        printf("Card reset timeout!\n");
        return false;
    }

    // This enabled VDD1 bus power for SD card, needed for RPI 4.
    unsigned int c0 = EMMC->control[0];
    c0 |= 0x0F << 8;
    EMMC->control[0] = c0;
    timer_sleep(3);

    if (!emmc_setup_clock())
    {
        return false;
    }

    // All interrupts go to interrupt register.
    EMMC->int_enable = 0;
    EMMC->int_flags = 0xFFFFFFFF;
    EMMC->int_mask = 0xFFFFFFFF;

    timer_sleep(203);

    device.transfer_blocks = 0;
    device.last_command_value = 0;
    device.last_success = false;
    device.block_size = 0;

    if (!emmc_command(CTGoIdle, 0, 2000))
    {
        printf("NO GO_IDLE RESPONSE\n");
        return false;
    }

    bool v2_card = check_v2_card();

    if (!check_usable_card())
    {
        return false;
    }

    if (!check_ocr())
    {
        return false;
    }

    if (!check_sdhc_support(v2_card))
    {
        return false;
    }

    switch_clock_rate(device.base_clock, SD_CLOCK_NORMAL);

    timer_sleep(10);

    if (!check_rca())
    {
        return false;
    }

    if (!select_card())
    {
        return false;
    }

    if (!set_scr())
    {
        return false;
    }

    // enable all interrupts
    EMMC->int_flags = 0xFFFFFFFF;

    if (EMMC_DEBUG)
        printf("Card reset!\n");

    return true;
}

int emmc_io_read(void *b, unsigned int size)
{
    return emmc_read((unsigned char *)b, size);
}

void emmc_io_seek(unsigned long offset)
{
    return emmc_seek(offset);
}

bool do_data_command(bool write, unsigned char *b, unsigned int bsize, unsigned int block_no)
{
    if (!device.sdhc)
    {
        block_no *= 512;
    }

    if (bsize < device.block_size)
    {
        printf("INVALID BLOCK SIZE: \n", bsize, device.block_size);
        return false;
    }

    device.transfer_blocks = bsize / device.block_size;

    if (bsize % device.block_size)
    {
        printf("BAD BLOCK SIZE\n");
        return false;
    }

    device.buffer = b;

    cmd_type command = CTReadBlock;

    if (write && device.transfer_blocks > 1)
    {
        command = CTWriteMultiple;
    }
    else if (write)
    {
        command = CTWriteBlock;
    }
    else if (!write && device.transfer_blocks > 1)
    {
        command = CTReadMultiple;
    }

    int retry_count = 0;
    int max_retries = 3;

    if (EMMC_DEBUG)
        printf("Sending command: %d\n", command);

    while (retry_count < max_retries)
    {
        if (emmc_command(command, block_no, 5000))
        {
            break;
        }

        if (++retry_count < max_retries)
        {
            printf("Retrying data command\n");
        }
        else
        {
            printf("Giving up data command\n");
            return false;
        }
    }

    return true;
}

int do_read(unsigned char *b, unsigned int bsize, unsigned int block_no)
{
    // TODO ENSURE DATA MODE...

    if (!do_data_command(false, b, bsize, block_no))
    {
        printf("do_data_command failed\n");
        return -1;
    }

    return bsize;
}

int do_write(unsigned char *b, unsigned int bsize, unsigned int block_no)
{
    if (!do_data_command(true, b, bsize, block_no))
    {
        printf("do_data_command with write failed\n");
        return -1;
    }

    return bsize;
}

int emmc_read(unsigned char *buffer, unsigned int size)
{
    if (device.offset % 512 != 0)
    {
        printf("INVALID OFFSET: %d\n", device.offset);
        return -1;
    }

    unsigned int block = device.offset / 512;

    int r = do_read(buffer, size, block);

    if (r != size)
    {
        printf("READ FAILED: %d\n", r);
        return -1;
    }

    return size;
}

int emmc_write(unsigned char *buffer, unsigned int size)
{
    if (device.offset % 512 != 0)
    {
        printf("INVALID OFFSET: %d\n", device.offset);
        return -1;
    }

    unsigned int block = device.offset / 512;

    int r = do_write(buffer, size, block);

    if (r != size)
    {
        printf("READ FAILED: %d\n", r);
        return -1;
    }

    return size;
}

void emmc_seek(unsigned long _offset)
{
    device.offset = _offset;
}

bool emmc_init()
{
    for (int i = 34; i <= 39; i++)
        gpio_set(i, GPIO_FUNCTION_IN);
    for (int i = 48; i <= 52; i++)
        gpio_setAlt3(i);

    device.transfer_blocks = 0;
    device.last_command_value = 0;
    device.last_success = false;
    device.block_size = 0;
    device.sdhc = false;
    device.ocr = 0;
    device.rca = 0;
    device.offset = 0;
    device.base_clock = 0;

    bool success = false;
    for (int i = 0; i < 10; i++)
    {
        success = emmc_card_reset();

        if (success)
        {
            break;
        }

        timer_sleep(100);
        printf("Failed to reset card, trying again...\n");
    }

    if (!success)
    {
        return false;
    }

    return true;
}