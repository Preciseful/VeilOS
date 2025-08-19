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

typedef unsigned long virtual_addr;
typedef unsigned long physical_addr;

void mmu_map_page(unsigned long *pgd, virtual_addr va, physical_addr pa, unsigned long index, enum MMU_Flags flags);
void mmu_unmap_page(unsigned long *pgd, virtual_addr va);
void mmu_map_block(unsigned long *pgd, virtual_addr va, physical_addr pa, unsigned long index, enum MMU_Flags flags);
void mmu_init();

extern void mmu_init_regs(unsigned long pgd, unsigned long high_pgd, unsigned long high_memory_i);
extern void refresh_ttbr(unsigned long pgd);

#endif