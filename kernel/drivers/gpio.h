/**
 * @file
 * @author Developful
 * @brief Interface for the GPIO.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <boot/base.h>
#include <stdbool.h>
#include <memory/mmu.h>

/**
 * @brief Actions on a GPIO pin.
 */
enum GPIO_Actions
{
    GPIOFSEL0 = PERIPHERAL_BASE + 0x200000,
    GPIOSET0 = PERIPHERAL_BASE + 0x20001C,
    GPIOCLR0 = PERIPHERAL_BASE + 0x200028,
    GPIOPUPPDN0 = PERIPHERAL_BASE + 0x2000E4
};

/**
 * @brief Pull options on a GPIO pin.
 */
enum GPIO_Pulls
{
    GPIO_PULL_NONE = 0,
    GPIO_PULL_DOWN = 2,
    GPIO_PULL_UP = 1
};

/**
 * @brief GPIO pin functions.
 */
enum GPIO
{
    GPIO_FUNCTION_IN = 0,
    GPIO_FUNCTION_OUT = 1,
    GPIO_FUNCTION_ALT5 = 2,
    GPIO_FUNCTION_ALT3 = 7,
    GPIO_FUNCTION_ALT0 = 4
};

/**
 * @brief Writes to a MMIO register.
 *
 * @param reg The virtual address of the register.
 * @param val The value to be written.
 */
void WriteToMMIO(VirtualAddr reg, unsigned int val);

/**
 * @brief Reads from a MMIO register.
 *
 * @param reg The virtual address of the register.
 * @return unsigned int The value at that register.
 */
unsigned int ReadMMIO(VirtualAddr reg);

/**
 * @brief Calls to perform an action on a GPIO pin.
 *
 * @param pin The pin number.
 * @param value The value for the action.
 * @param base The action to be done.
 * @param field_size The number of bits per pin.
 * @return `true` if the action was successful.
 * @return `false` if the \p pin number or \p value were too large.
 */
bool CallGPIO(unsigned long pin, unsigned long value, enum GPIO_Actions base, unsigned long field_size);

/**
 * @brief Calls the GPIOSET0 action on the pin.
 * Writes to the GPSET0 register to set a GPIO pin HIGH.
 * @param pin The pin number.
 * @param value The value for the action.
 * @return `true` if the action was successful.
 * @return `false` if the \p pin number or \p value were too large.
 */
bool SetGPIO(unsigned long pin, unsigned long value);

/**
 * @brief Calls the GPIOCLR0 action on the pin.
 * Writes to the GPCLR0 register to set a GPIO pin LOW.
 * @param pin The pin number.
 * @param value The value for the action.
 * @return `true` if the action was successful.
 * @return `false` if the \p pin number or \p value were too large.
 */
bool ClearGPIO(unsigned long pin, unsigned long value);

/**
 * @brief Calls the GPIOPUPPDN0 action on the pin.
 * This sets the pull value of the GPIO pin to none, up or down.
 * @param pin The pin number.
 * @param pull The pull value for the action.
 * @return `true` if the action was successful.
 * @return `false` if the \p pin number or \p pull were too large.
 */
bool PullGPIO(unsigned long pin, enum GPIO_Pulls pull);

/**
 * @brief Calls the GPIOFSEL0 action on the pin.
 * This sets the function value of the pin: in, out, alt0, alt3, or alt5.
 * @param pin The pin number.
 * @param function The function value for the action.
 * @return `true` if the action was successful.
 * @return `false` if the \p pin number or \p function value were too large.
 */
bool SetGPIOFunction(unsigned long pin, enum GPIO function);

/**
 * @brief Sets the function value to alt0.
 * @param pin The pin number.
 */
void SetAlt0(unsigned long pin);

/**
 * @brief Sets the function value to alt3.
 * @param pin The pin number.
 */
void SetAlt3(unsigned long pin);

/**
 * @brief Sets the function value to alt5.
 * @param pin The pin number.
 */
void SetAlt5(unsigned long pin);

/**
 * @brief Sets the pin to HIGH or LOW depending on the boolean value.
 *
 * @param pin The pin number.
 * @param value The boolean value: high (1) or low (0).
 */
void SetPinOutputBool(unsigned long pin, bool value);