/**
 * @author Developful
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

.global refresh_ttbr
refresh_ttbr:
    msr ttbr1_el1, x0
    msr ttbr0_el1, xzr

    tlbi vmalle1is
	dsb ish
	isb

    ret

.global reset_va
reset_va:
    tlbi vaae1is, x0
    dsb ish
    isb
    ret