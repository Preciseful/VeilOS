/**
 * @file
 * @author Developful
 * @brief Error values for functions.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

/**
 * @brief All members should be called with a minus, in order to turn them into a negative value.
 */
enum
{
    /// @brief Left blank, 0 cannot be negative.
    E_BLANK,
    /// @brief The requested file cannot be found.
    E_NO_FILE,
    /// @brief The requested mount point cannot be found.
    E_NO_MOUNT,
    /// @brief The operation called is invalid, no such operation is registered or exists.
    E_INVALID_OPERATION,
    /// @brief The requested IO device cannot be found.
    E_NO_IO_DEVICE,
};