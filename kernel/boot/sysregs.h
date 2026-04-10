/**
 * @file
 * @author Developful
 * @brief System register values.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#define SCTLR_EL1_RESERVED (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
#define SCTLR_EL1_EE_LITTLE_ENDIAN (0 << 25)
#define SCTLR_EL1_EOE_LITTLE_ENDIAN (0 << 24)
#define SCTLR_EL1_I_CACHE_DISABLED (0 << 12)
#define SCTLR_EL1_D_CACHE_DISABLED (0 << 2)
#define SCTLR_EL1_I_CACHE_ENABLED (1 << 12)
#define SCTLR_EL1_D_CACHE_ENABLED (1 << 2)
#define SCTLR_EL1_MMU_DISABLED (0 << 0)
#define SCTLR_EL1_MMU_ENABLED (1 << 0)
#define SCTLR_EL1_VALUE_MMU_DISABLED (SCTLR_EL1_RESERVED | SCTLR_EL1_EE_LITTLE_ENDIAN | SCTLR_EL1_I_CACHE_ENABLED | SCTLR_EL1_D_CACHE_ENABLED | SCTLR_EL1_MMU_DISABLED)

#define HCR_EL2_RW (1 << 31)
#define HCR_EL2_VALUE HCR_EL2_RW

#define SCR_EL3_RESERVED (3 << 4)
#define SCR_EL3_RW (1 << 10)
#define SCR_EL3_NS (1 << 0)
#define SCR_EL3_VALUE (SCR_EL3_RESERVED | SCR_EL3_RW | SCR_EL3_NS)

#define SPSR_EL3_MASK_ALL (7 << 6)
#define SPSR_EL3_EL1h (5 << 0)
#define SPSR_EL3_VALUE (SPSR_EL3_MASK_ALL | SPSR_EL3_EL1h)