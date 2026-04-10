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