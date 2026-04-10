/**
 * @file
 * @author Developful
 * @brief Kernel stack tracing interface.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

/**
 * @brief Caches the kernel symbols in memory for tracing.
 */
void CacheKernelSymbols();

/**
 * @brief Starts tracing from a certain stack pointer.
 *
 * @param x29 The stack pointer.
 */
void Trace(unsigned long x29);