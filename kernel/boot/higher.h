/**
 * @file
 * @author Developful
 * @brief Higher VA jump helpers.
 * @date 2026-09-18
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

extern void jump_high(unsigned long pgd, unsigned long high_pgd, unsigned long last_page);

/**
 * @brief Initiate the jump to high VA.
 *
 * @param dtb The dtb address passed at boot.
 */
void JumpToHigher(void *dtb);