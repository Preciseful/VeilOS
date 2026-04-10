/**
 * @file
 * @author Developful
 * @brief Interface for the raspberry pi 4 mailbox.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>
#include <boot/base.h>

#define MAILBOX_BASE PERIPHERAL_BASE + 0xB880
#define MAILBOX_READ MAILBOX_BASE + 0x0
#define MAILBOX_POLL MAILBOX_BASE + 0x10
#define MAILBOX_SENDER MAILBOX_BASE + 0x14
#define MAILBOX_STATUS MAILBOX_BASE + 0x18
#define MAILBOX_CONFIG MAILBOX_BASE + 0x1C
#define MAILBOX_WRITE MAILBOX_BASE + 0x20

#define MAILBOX_REQUEST 0x0
#define MAILBOX_RESPONSE 0x80000000
#define MAILBOX_SUCCESS 0x80000000
#define MAILBOX_FAIL 0x80000001
#define MAILBOX_FULL 0x80000000
#define MAILBOX_EMPTY 0x40000000

enum Mailbox_Clocks
{
    EMMC_CLOCK = 1
};

enum Mailbox_Channels
{
    POWER_MANAGEMENT_CHANNEL,
    FRAMEBUFFER_CHANNEL,
    VIRTUAL_UART_CHANNEL,
    VCHIQ_CHANNEL,
    LEDS_CHANNEL,
    BUTTONS_CHANNEL,
    TOUCH_SCR_EL3EEN_CHANNEL,
    ARM_TO_GPU,
    GPU_TO_ARM
};

/**
 * @brief The mailbox array used when sending a message to channels.
 */
extern volatile unsigned int mailbox[36];

/**
 * @brief Get the rate of a clock.
 *
 * @param clock The clock value.
 * @return The rate of the clock.
 */
unsigned int GetMailboxClockRate(enum Mailbox_Clocks clock);

/**
 * @brief Sends a message to a mailbox channel, using the `mailbox` array for the messages.
 *
 * @param channel The channel value.
 * @return `true` if there was a response, otherwise `false`.
 */
bool CallMailbox(unsigned char channel);