#include <memory/mmu.h>
#include <memory/memory.h>
#include <boot/base.h>
#include <lib/printf.h>
#include <drivers/uart.h>

#define PAGE_ENTRIES 512

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_ACCESS (1 << 10)

#define PAGE_TABLE_ENTRIES 512
#define PAGE_MASK 0xFFFFFFFFF000ULL

void MapTablePage(unsigned long *pgd, VirtualAddr va, PhysicalAddr pa, unsigned long index, enum MMU_Flags flags)
{
    unsigned long l1_index = (va >> 39) & 0x1FF;
    unsigned long l2_index = (va >> 30) & 0x1FF;
    unsigned long l3_index = (va >> 21) & 0x1FF;
    unsigned long pte_index = (va >> 12) & 0x1FF;

    if (!(pgd[l1_index] & 1))
    {
        unsigned long *l1 = malloc(PAGE_SIZE);
        memset(l1, 0, PAGE_SIZE);
        pgd[l1_index] = (VIRT_TO_PHYS(l1) & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l1 = (unsigned long *)PHYS_TO_VIRT((pgd[l1_index] & PAGE_MASK));

    if (!(l1[l2_index] & 1))
    {
        unsigned long *l2 = malloc(PAGE_SIZE);
        memset(l2, 0, PAGE_SIZE);
        l1[l2_index] = (VIRT_TO_PHYS(l2) & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l2 = (unsigned long *)PHYS_TO_VIRT((l1[l2_index] & PAGE_MASK));

    if (!(l2[l3_index] & 1))
    {
        unsigned long *l3 = malloc(PAGE_SIZE);
        memset(l3, 0, PAGE_SIZE);
        l2[l3_index] = (VIRT_TO_PHYS(l3) & PAGE_MASK) | PD_TABLE;
    }
    else if ((l2[l3_index] & 0b11) == PD_BLOCK)
    {
        LOG("Page occupied by a block.\n");
        return;
    }

    unsigned long *l3 = (unsigned long *)PHYS_TO_VIRT((l2[l3_index] & PAGE_MASK));

    unsigned char ng = 0;
    bool uxn = !((flags & MMU_USER_EXEC) >> 2);
    bool pxn = !uxn;
    unsigned long perm = flags & 0b11;
    if (perm == MMU_RR || perm == MMU_RWRW)
        ng = 1;
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
        return;

    unsigned long *l1 = (unsigned long *)PHYS_TO_VIRT((pgd[l1_index] & PAGE_MASK));
    if (!(l1[l2_index] & 1))
        return;

    unsigned long *l2 = (unsigned long *)PHYS_TO_VIRT((l1[l2_index] & PAGE_MASK));
    if (!(l2[l3_index] & 1))
        return;

    unsigned long *l3 = (unsigned long *)PHYS_TO_VIRT((l2[l3_index] & PAGE_MASK));
    if (!(l3[pte_index] & 1))
        return;

    l3[pte_index] = 0;
}

void MapTableBlock(unsigned long *pgd, VirtualAddr va, PhysicalAddr pa, unsigned long index, enum MMU_Flags flags)
{
    unsigned long l1_index = (va >> 39) & 0x1FF;
    unsigned long l2_index = (va >> 30) & 0x1FF;
    unsigned long l3_index = (va >> 21) & 0x1FF;

    if (!(pgd[l1_index] & 1))
    {
        unsigned long *l1 = malloc(PAGE_SIZE);
        memset(l1, 0, PAGE_SIZE);
        pgd[l1_index] = (VIRT_TO_PHYS(l1) & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l1 = (unsigned long *)PHYS_TO_VIRT((pgd[l1_index] & PAGE_MASK));

    if (!(l1[l2_index] & 1))
    {
        unsigned long *l2 = malloc(PAGE_SIZE);
        memset(l2, 0, PAGE_SIZE);
        l1[l2_index] = (VIRT_TO_PHYS(l2) & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l2 = (unsigned long *)PHYS_TO_VIRT((l1[l2_index] & PAGE_MASK));

    bool uxn = !((flags & MMU_USER_EXEC) >> 2);
    bool pxn = !uxn;
    unsigned long perm = flags & 0b11;

    unsigned long attr = ((unsigned long)uxn << 54) | ((unsigned long)pxn << 53) | PD_ACCESS | (0b11 << 8) | (perm << 6) | (index << 2) | PD_BLOCK;

    l2[l3_index] = (pa & 0xFFFFFFFFF000ULL) | attr;
}

void *temp_malloc(unsigned long i)
{
    unsigned long adr = LOW_MEMORY + i * PAGE_SIZE;
    return (void *)adr;
}

unsigned long mmu_map_temp_block(unsigned long *pgd, unsigned long va, unsigned long pa, unsigned long index, bool kernel, unsigned long malloc_i)
{
    unsigned long l1_index = (va >> 39) & 0x1FF;
    unsigned long l2_index = (va >> 30) & 0x1FF;
    unsigned long l3_index = (va >> 21) & 0x1FF;

    if (!(pgd[l1_index] & 1))
    {
        unsigned long *l1 = temp_malloc(malloc_i);
        malloc_i++;
        memset(l1, 0, PAGE_SIZE);
        pgd[l1_index] = ((unsigned long)l1 & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l1 = (unsigned long *)(pgd[l1_index] & PAGE_MASK);

    if (!(l1[l2_index] & 1))
    {
        unsigned long *l2 = temp_malloc(malloc_i);
        malloc_i++;
        memset(l2, 0, PAGE_SIZE);
        l1[l2_index] = ((unsigned long)l2 & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l2 = (unsigned long *)(l1[l2_index] & PAGE_MASK);
    unsigned char perm = kernel ? 0b00 : 0b01;

    unsigned long attr = ((unsigned long)1 << 54) | ((unsigned long)0 << 53) | PD_ACCESS | (0b11 << 8) | (perm << 6) | (index << 2) | PD_BLOCK;

    l2[l3_index] = (pa & 0xFFFFFFFFF000ULL) | attr;
    return malloc_i;
}

void MMUInit()
{
    // retain the amount of memory allocated in total
    unsigned long high_memory_i = 0;
    // 20 is just random, hopefully no way high_memory_i reaches it
    unsigned long low_memory_i = 20;
    unsigned long *pgd = temp_malloc(low_memory_i);
    low_memory_i++;
    unsigned long *high_pgd = temp_malloc(high_memory_i);
    high_memory_i++;

    // make 2 maps: a lower half and a higher half
    for (unsigned long addr = 0; addr <= GRANULE_1GB * 4; addr += GRANULE_2MB)
    {
        if (addr < DEVICE_START)
        {
            high_memory_i = mmu_map_temp_block(high_pgd, HIGH_VA + addr, addr, MAIR_IDX_NORMAL, true, high_memory_i);
            low_memory_i = mmu_map_temp_block(pgd, addr, addr, MAIR_IDX_NORMAL, true, low_memory_i);
        }
        else
        {
            high_memory_i = mmu_map_temp_block(high_pgd, HIGH_VA + addr, addr, MAIR_IDX_DEVICE, true, high_memory_i);
            low_memory_i = mmu_map_temp_block(pgd, addr, addr, MAIR_IDX_DEVICE, true, low_memory_i);
        }
    }

    // init the regs with both halves
    mmu_init_regs((unsigned long)pgd, (unsigned long)high_pgd, high_memory_i);
}

extern void kmain();

void finish_higher(unsigned long high_memory_i)
{
    MMInit();
    malloc(PAGE_SIZE * high_memory_i);

    // make ttbr0 invalid, use only ttbr1
    refresh_ttbr((unsigned long)temp_malloc(0));

    kmain();
}