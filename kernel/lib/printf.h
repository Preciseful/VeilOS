/**
 * @file
 * @author Developful
 * @brief Printing with format functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#pragma GCC diagnostic ignored "-Wmultistatement-macros"

#include <stdarg.h>

#define PRINTF_MAGIC_PADDING "                                       "
#define LOG(x, ...)                                                                                                                           \
    Printf("[%s/%s]%s" x, __FILE_NAME__, __func__, PRINTF_MAGIC_PADDING + (sizeof(__func__) - 1 + sizeof(__FILE_NAME__) - 1), ##__VA_ARGS__); \
    _Static_assert(sizeof(__FILE_NAME__) - 1 + sizeof(__func__) - 1 <= sizeof(PRINTF_MAGIC_PADDING), "Function and file name too long for printing.");

/**
 * @brief Sets the function that printf uses to print.
 * @param p The function pointer.
 */
void SetPrintf(void (*p)(char c));
unsigned int SPrintf(char *dst, char *fmt, ...);
void Printf(char *fmt, ...);