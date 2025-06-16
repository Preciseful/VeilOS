#pragma once

#define MT_DEVICE_nGnRnE_INDEX 0
#define MT_NORMAL_NC_INDEX 1
#define MT_DEVICE_nGnRnE 0x0
#define MT_NORMAL_NC 0x1
#define MT_DEVICE_nGnRnE_FLAGS 0x00
#define MT_NORMAL_NC_FLAGS 0x44
#define MAIR_VALUE (MT_DEVICE_nGnRnE_FLAGS << (8 * MT_DEVICE_nGnRnE)) | (MT_NORMAL_NC_FLAGS << (8 * MT_NORMAL_NC))

#define TCR_TG1_4K (2 << 30)
#define TCR_T1SZ ((64 - 48) << 16)
#define TCR_TG0_4K (0 << 14)
#define TCR_T0SZ (64 - 48)
#define TCR_VALUE (TCR_TG1_4K | TCR_T1SZ | TCR_TG0_4K | TCR_T0SZ)

#define CPACR_FPEN (1 << 21) | (1 << 20)
#define CPACR_ZEN (1 << 17) | (1 << 16)
#define CPACR_VALUE (CPACR_FPEN | CPACR_ZEN)

#ifndef __ASSEMBLER__

#include <memory/memory.h>

#define TD_VALID (1 << 0)
#define TD_BLOCK (0 << 1)
#define TD_TABLE (1 << 1)
#define TD_ACCESS (1 << 10)
#define TD_KERNEL_PERMS (1L << 54)
#define TD_INNER_SHARABLE (3 << 8)

#define MATTR_DEVICE_nGnRnE_INDEX 0
#define MATTR_NORMAL_NC_INDEX 1

#define TD_KERNEL_TABLE_FLAGS (TD_TABLE | TD_VALID)
#define TD_KERNEL_BLOCK_FLAGS (TD_ACCESS | TD_INNER_SHARABLE | TD_KERNEL_PERMS | (MATTR_NORMAL_NC_INDEX << 2) | TD_BLOCK | TD_VALID)
#define TD_DEVICE_BLOCK_FLAGS (TD_ACCESS | TD_INNER_SHARABLE | TD_KERNEL_PERMS | (MATTR_DEVICE_nGnRnE_INDEX << 2) | TD_BLOCK | TD_VALID)

#define BLOCK_SIZE 0x40000000

#define TABLE_ENTRIES 512
#define PGD_SHIFT (PAGE_SHIFT + 3 * TABLE_SHIFT)
#define PUD_SHIFT (PAGE_SHIFT + 2 * TABLE_SHIFT)
#define PMD_SHIFT (PAGE_SHIFT + TABLE_SHIFT)
#define PTE_SHIFT (PAGE_SHIFT)

void make_entry(unsigned long table, unsigned long entry, unsigned long va, unsigned long shift, unsigned long flags);
void make_block(unsigned long pmd, unsigned long vstart, unsigned long vend, unsigned long pa, unsigned long shift);

#endif