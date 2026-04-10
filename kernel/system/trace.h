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