#pragma once

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b00000000
#define MAIR_IDX_DEVICE 0
#define MAIR_IDX_NORMAL 1

#define MAIR_VALUE (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE * 8)) | (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL * 8))
#define TCR_VALUE (((64 - 48) << 0) | ((64 - 48) << 16) | (0b00 << 14) | (0b10 << 30))

#ifndef __ASSEMBLER__

#include <memory/memory.h>

#define GRANULE_4KB 0x1000
#define GRANULE_2MB 0x200000
#define GRANULE_1GB 0x40000000UL

enum MMU_Flags
{
    MMU_USER_EXEC = 0b100,
    MMU_NORW = 0b00,
    MMU_RWRW = 0b01,
    MMU_NOR = 0b10,
    MMU_RR = 0b11
};

void mmu_map_page(unsigned long *table, unsigned long va, unsigned long pa, unsigned long index, enum MMU_Flags flags);
void mmu_map_block(unsigned long *pgd, unsigned long va, unsigned long pa, unsigned long index, enum MMU_Flags flags);
void mmu_init();
void debug_mmu_address(unsigned long *pgd, unsigned long va);

extern void mmu_init_regs(unsigned long pgd, unsigned long high_pgd, unsigned long high_memory_i);
extern void refresh_ttbr(unsigned long pgd);

#endif