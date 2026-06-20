/**
 * @author Developful
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <memory/mmu.h>
#include <memory/allocator.h>
#include <boot/base.h>
#include <lib/printf.h>
#include <drivers/uart.h>
#include <lib/string.h>
#include <memory/pmm.h>

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_ACCESS (1 << 10)

#define PAGE_TABLE_ENTRIES 512
#define PAGE_MASK 0xFFFFFFFFF000ULL

PhysicalAddr allocate_new_table(unsigned long *pgd, unsigned long **ln)
{
    if ((unsigned long)pgd == HIGH_VA + LOW_MEMORY)
    {
        PhysicalAddr phys = GetPhysicalPage();
        MapTablePage(pgd, HIGH_VA, phys, MAIR_IDX_NORMAL, MMU_NORW);
        *ln = (unsigned long *)HIGH_VA;
        return phys;
    }

    *ln = malloc(PAGE_SIZE);
    return VIRT_TO_PHYS(*ln);
}

void MapTablePage(unsigned long *pgd, VirtualAddr va, PhysicalAddr pa, unsigned long index, enum MMU_Flags flags)
{
    unsigned long l1_index = (va >> 39) & 0x1FF;
    unsigned long l2_index = (va >> 30) & 0x1FF;
    unsigned long l3_index = (va >> 21) & 0x1FF;
    unsigned long pte_index = (va >> 12) & 0x1FF;

    if (!(pgd[l1_index] & 1))
    {
        unsigned long *l1;
        PhysicalAddr phys = allocate_new_table(pgd, &l1);

        memset(l1, 0, PAGE_SIZE);
        pgd[l1_index] = (phys & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l1 = (unsigned long *)PHYS_TO_VIRT((pgd[l1_index] & PAGE_MASK));

    if (!(l1[l2_index] & 1))
    {
        unsigned long *l2;
        PhysicalAddr phys = allocate_new_table(pgd, &l2);

        memset(l2, 0, PAGE_SIZE);
        l1[l2_index] = (phys & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l2 = (unsigned long *)PHYS_TO_VIRT((l1[l2_index] & PAGE_MASK));

    if (!(l2[l3_index] & 1))
    {
        unsigned long *l3;
        PhysicalAddr phys = allocate_new_table(pgd, &l3);

        memset(l3, 0, PAGE_SIZE);
        l2[l3_index] = (phys & PAGE_MASK) | PD_TABLE;
    }
    else if ((l2[l3_index] & 0b11) == PD_BLOCK)
    {
        LOG("Page occupied by a block.\n");
        return;
    }

    unsigned long *l3 = (unsigned long *)PHYS_TO_VIRT((l2[l3_index] & PAGE_MASK));

    bool uxn = !((flags & MMU_USER_EXEC) >> 2);
    bool pxn = !uxn;

    unsigned char ng = 1;
    unsigned long perm = flags & 0b11;

    unsigned long attr = ((unsigned long)uxn << 54) | ((unsigned long)pxn << 53) | (ng << 11) | PD_ACCESS | (0b11 << 8) | (perm << 6) | (index << 2) | 0b11;
    if (l3[pte_index] & 1)
    {
        LOG("[MMU warning]: Section already mapped (%x).\n", va);
        return;
    }

    l3[pte_index] = (pa & PAGE_MASK) | attr;
}

void UnmapTablePage(unsigned long *pgd, VirtualAddr va)
{
    unsigned long l1_index = (va >> 39) & 0x1FF;
    unsigned long l2_index = (va >> 30) & 0x1FF;
    unsigned long l3_index = (va >> 21) & 0x1FF;
    unsigned long pte_index = (va >> 12) & 0x1FF;

    if (!(pgd[l1_index] & 1))
    {
        LOG("VA %lx Not found at L1.\n", va);
        return;
    }

    unsigned long *l1 = (unsigned long *)PHYS_TO_VIRT((pgd[l1_index] & PAGE_MASK));
    if (!(l1[l2_index] & 1))
    {
        LOG("VA %lx Not found at L2.\n", va);
        return;
    }

    unsigned long *l2 = (unsigned long *)PHYS_TO_VIRT((l1[l2_index] & PAGE_MASK));
    if (!(l2[l3_index] & 1))
    {
        LOG("VA %lx Not found at L3.\n", va);
        return;
    }

    unsigned long *l3 = (unsigned long *)PHYS_TO_VIRT((l2[l3_index] & PAGE_MASK));
    if (!(l3[pte_index] & 1))
    {
        LOG("VA %lx Not found at PTE.\n", va);
        return;
    }

    l3[pte_index] = 0;
}

void FreeTable(unsigned long *table, unsigned int level)
{
    for (unsigned long i = 0; i < 512; i++)
    {
        if (!(table[i] & 1))
            continue;

        unsigned long type = table[i] & 0x3;
        unsigned long *child = (unsigned long *)PHYS_TO_VIRT((table[i] & PAGE_MASK));

        if (type == PD_TABLE && level < 3)
            FreeTable(child, level + 1);
        else
            table[i] = 0;

        free(child);
    }

    if (level == 0)
        free(table);
}