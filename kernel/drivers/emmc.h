#ifndef EMMC_H
#define EMMC_H

#include <stdbool.h>
#include <stdint.h>
#include <boot/base.h>

// HUGE THANKS to https://github.com/rockytriton/LLD/blob/main/rpi_bm/part17

#define PACKED __attribute((__packed__))

#define EMMC_DEBUG 0

#define BSWAP32(x) (((x << 24) & 0xff000000) | \
                    ((x << 8) & 0x00ff0000) |  \
                    ((x >> 8) & 0x0000ff00) |  \
                    ((x >> 24) & 0x000000ff))

typedef struct
{
    unsigned char resp_a : 1;
    unsigned char block_count : 1;
    unsigned char auto_command : 2;
    unsigned char direction : 1;
    unsigned char multiblock : 1;
    unsigned short resp_b : 10;
    unsigned char response_type : 2;
    unsigned char res0 : 1;
    unsigned char crc_enable : 1;
    unsigned char idx_enable : 1;
    unsigned char is_data : 1;
    unsigned char type : 2;
    unsigned char index : 6;
    unsigned char res1 : 2;
} emmc_cmd;

#define RES_CMD {1, 1, 3, 1, 1, 0xF, 3, 1, 1, 1, 1, 3, 0xF, 3}

typedef enum
{
    RTNone,
    RT136,
    RT48,
    RT48Busy
} cmd_resp_type;

typedef enum
{
    CTGoIdle = 0,
    CTSendCide = 2,
    CTSendRelativeAddr = 3,
    CTIOSetOpCond = 5,
    CTSelectCard = 7,
    CTSendIfCond = 8,
    CTSetBlockLen = 16,
    CTReadBlock = 17,
    CTReadMultiple = 18,
    CTWriteBlock = 24,
    CTWriteMultiple = 25,
    CTOcrCheck = 41,
    CTSendSCR = 51,
    CTApp = 55
} cmd_type;

typedef struct
{
    unsigned int scr[2];
    unsigned int bus_widths;
    unsigned int version;
} scr_register;

typedef enum
{
    SDECommandTimeout,
    SDECommandCrc,
    SDECommandEndBit,
    SDECommandIndex,
    SDEDataTimeout,
    SDEDataCrc,
    SDEDataEndBit,
    SDECurrentLimit,
    SDEAutoCmd12,
    SDEADma,
    SDETuning,
    SDERsvd
} sd_error;

typedef struct
{
    bool last_success;
    unsigned int transfer_blocks;
    emmc_cmd last_command;
    volatile unsigned int last_command_value;
    unsigned int block_size;
    unsigned int last_response[4];
    bool sdhc;
    unsigned short ocr;
    unsigned int rca;
    unsigned long offset;
    void *buffer;
    unsigned int base_clock;
    unsigned int last_error;
    unsigned int last_interrupt;
    scr_register scr;
} emmc_device;

typedef struct
{
    volatile unsigned int arg2;
    volatile unsigned int block_size_count;
    volatile unsigned int arg1;
    volatile unsigned int cmd_xfer_mode;
    volatile unsigned int response[4];
    volatile unsigned int data;
    volatile unsigned int status;
    volatile unsigned int control[2];
    volatile unsigned int int_flags;
    volatile unsigned int int_mask;
    volatile unsigned int int_enable;
    volatile unsigned int control2;
    volatile unsigned int cap1;
    volatile unsigned int cap2;
    volatile unsigned int res0[2];
    volatile unsigned int force_int;
    volatile unsigned int res1[7];
    volatile unsigned int boot_timeout;
    volatile unsigned int debug_config;
    volatile unsigned int res2[2];
    volatile unsigned int ext_fifo_config;
    volatile unsigned int ext_fifo_enable;
    volatile unsigned int tune_step;
    volatile unsigned int tune_SDR;
    volatile unsigned int tune_DDR;
    volatile unsigned int res3[23];
    volatile unsigned int spi_int_support;
    volatile unsigned int res4[2];
    volatile unsigned int slot_int_status;
} emmc_regs;

#define TO_REG(p) *((volatile unsigned int *)p)

// SD Clock Frequencies (in Hz)
#define SD_CLOCK_ID 400000
#define SD_CLOCK_NORMAL 25000000
#define SD_CLOCK_HIGH 50000000
#define SD_CLOCK_100 100000000
#define SD_CLOCK_208 208000000
#define SD_COMMAND_COMPLETE 1
#define SD_TRANSFER_COMPLETE (1 << 1)
#define SD_BLOCK_GAP_EVENT (1 << 2)
#define SD_DMA_INTERRUPT (1 << 3)
#define SD_BUFFER_WRITE_READY (1 << 4)
#define SD_BUFFER_READ_READY (1 << 5)
#define SD_CARD_INSERTION (1 << 6)
#define SD_CARD_REMOVAL (1 << 7)
#define SD_CARD_INTERRUPT (1 << 8)
#define EMMC_BASE (PERIPHERAL_BASE + 0x00340000)

#define EMMC ((emmc_regs *)EMMC_BASE)

bool emmc_init();
int emmc_read(unsigned char *buffer, unsigned int size);
int emmc_write(unsigned char *buffer, unsigned int size);
void emmc_seek(unsigned long offset);
bool emmc_command(unsigned int command, unsigned int arg, unsigned int timeout);

#define EMMC_CTRL1_RESET_DATA (1 << 26)
#define EMMC_CTRL1_RESET_CMD (1 << 25)
#define EMMC_CTRL1_RESET_HOST (1 << 24)
#define EMMC_CTRL1_RESET_ALL (EMMC_CTRL1_RESET_DATA | EMMC_CTRL1_RESET_CMD | EMMC_CTRL1_RESET_HOST)

#define EMMC_CTRL1_CLK_GENSEL (1 << 5)
#define EMMC_CTRL1_CLK_ENABLE (1 << 2)
#define EMMC_CTRL1_CLK_STABLE (1 << 1)
#define EMMC_CTRL1_CLK_INT_EN (1 << 0)

#define EMMC_CTRL0_ALT_BOOT_EN (1 << 22)
#define EMMC_CTRL0_BOOT_EN (1 << 21)
#define EMMC_CTRL0_SPI_MODE (1 << 20)

#define EMMC_STATUS_DAT_INHIBIT (1 << 1)
#define EMMC_STATUS_CMD_INHIBIT (1 << 0)

#endif // EMMC_H
