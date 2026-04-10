/**
 * @file
 * @author Developful
 * @brief Panic interfacing for the OS.
 * @date 2026-04-10
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

/**
 * @brief Panic function, halting the entire OS.
 *
 * @param message The message to show when halting.
 */
[[noreturn]] void panic(const char *message);