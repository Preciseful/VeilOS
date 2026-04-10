#pragma once

#include <system/syscall.h>

/**
 * @brief The amount of characters read to be maintained.
 */
#define READ_BUF_SIZE 1024

/**
 * @brief Initialize the UART0.
 */
void UartInit();

/**
 * @brief Adds the last received UART character to a buffer.
 * The buffer maintains the last READ_BUF_SIZE characters read.
 */
void HandleUartReceive();