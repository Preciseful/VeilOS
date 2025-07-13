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

void mmu_map_page(unsigned long *table, unsigned long va, unsigned long pa, unsigned long index, bool kernel);
void mmu_map_block(unsigned long *pgd, unsigned long va, unsigned long pa, unsigned long index, bool kernel);
void mmu_init();
void debug_mmu_address(unsigned long *pgd, unsigned long va);

extern void mmu_init_regs(unsigned long pgd);

#endif