/**
 * @file
 * @author Developful
 * @brief Interface for the random number generator on raspberry pi 4.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

/**
 * @brief Initializes the hardware random number generator.
 */
void RNGInit();

/**
 * @return A randomly generated unsigned 32-bit integer value.
 */
unsigned int GetRandom32();

/**
 * @return A randomly generated unsigned 64-bit integer value.
 */
unsigned long GetRandom64();