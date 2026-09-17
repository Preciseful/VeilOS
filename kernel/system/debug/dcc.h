/**
 * @file
 * @author Developful
 * @brief ARM DCC communication helpers.
 * @date 2026-09-18
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

extern void tell_host(unsigned int operation, void *value);

/**
 * @brief Writes output through the DCC channel.
 */
void DebugWrite(char *fmt, ...);