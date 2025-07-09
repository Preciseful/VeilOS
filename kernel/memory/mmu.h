#pragma once
#ifndef __ASSEMBLER__

#include <memory/memory.h>

#define MAIR_IDX_DEVICE 0
#define MAIR_IDX_NORMAL 1

#define GRANULE_4KB 0x1000
#define GRANULE_2MB 0x200000
#define GRANULE_1GB 0x40000000UL

void mmu_map_page(unsigned long *table, unsigned long va, unsigned long pa, unsigned long index);
void mmu_map_block(unsigned long *pgd, unsigned long va, unsigned long pa, unsigned long index);
void mmu_init();
void debug_mmu_address(unsigned long *pgd, unsigned long va);

#endif