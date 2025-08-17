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

void debug_mmu_address(unsigned long *pgd, unsigned long va)
{
    unsigned long l1_index = (va >> 39) & 0x1FF;
    unsigned long l2_index = (va >> 30) & 0x1FF;
    unsigned long l3_index = (va >> 21) & 0x1FF;
    unsigned long pte_index = (va >> 12) & 0x1FF;

    LOG("Address is meant to be mapped to [%d][%d][%d][%d].\n", l1_index, l2_index, l3_index, pte_index);

    if (!(pgd[l1_index] & 1))
    {
        LOG("L1 Table missing!\n");
        return;
    }

    unsigned long *l1 = (unsigned long *)(pgd[l1_index] & 0xFFFFFFFFF000ULL);

    if (!(l1[l2_index] & 1))
    {
        LOG("L2 Table missing!\n");
        return;
    }

    unsigned long *l2 = (unsigned long *)(l1[l2_index] & 0xFFFFFFFFF000ULL);
    unsigned long l2_val = l2[l3_index];

    if (!(l2_val & 1))
    {
        LOG("L3 Table missing!\n");
        return;
    }

    if (!((l2_val >> 1) & 1))
    {
        LOG("Mapped as 2MB memory in L2!\n");
        LOG("Entry: %x\n", l2_val);
        return;
    }

    unsigned long *l3 = (unsigned long *)(l2[l3_index] & 0xFFFFFFFFF000ULL);
    unsigned long l3_val = l3[pte_index];

    if (!(l3_val & 1))
    {
        LOG("PTE missing!\n");
        return;
    }

    LOG("Entry: %x\n", l3_val);
    return;
}

void mmu_map_page(unsigned long *table, unsigned long va, unsigned long pa, unsigned long index, enum MMU_Flags flags)
{
    unsigned long l1_index = (va >> 39) & 0x1FF;
    unsigned long l2_index = (va >> 30) & 0x1FF;
    unsigned long l3_index = (va >> 21) & 0x1FF;
    unsigned long pte_index = (va >> 12) & 0x1FF;

    if (!(table[l1_index] & 1))
    {
        unsigned long *l1 = malloc(PAGE_SIZE);
        memset(l1, 0, PAGE_SIZE);
        table[l1_index] = (VIRT_TO_PHYS((unsigned long)l1) & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l1 = (unsigned long *)PHYS_TO_VIRT((table[l1_index] & PAGE_MASK));

    if (!(l1[l2_index] & 1))
    {
        unsigned long *l2 = malloc(PAGE_SIZE);
        memset(l2, 0, PAGE_SIZE);
        l1[l2_index] = (VIRT_TO_PHYS((unsigned long)l2) & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l2 = (unsigned long *)PHYS_TO_VIRT((l1[l2_index] & PAGE_MASK));

    if (!(l2[l3_index] & 1))
    {
        unsigned long *l3 = malloc(PAGE_SIZE);
        memset(l3, 0, PAGE_SIZE);
        l2[l3_index] = (VIRT_TO_PHYS((unsigned long)l3) & PAGE_MASK) | PD_TABLE;
    }
    else if ((l2[l3_index] & 0b11) == PD_BLOCK)
    {
        LOG("Page occupied by a block.\n");
        return;
    }

    unsigned long *l3 = (unsigned long *)PHYS_TO_VIRT((l2[l3_index] & PAGE_MASK));

    bool uxn = !((flags & MMU_USER_EXEC) >> 2);
    bool pxn = !uxn;
    unsigned long perm = flags & 0b11;

    unsigned long attr = ((unsigned long)uxn << 54) | ((unsigned long)pxn << 53) | PD_ACCESS | (0b11 << 8) | (perm << 6) | (index << 2) | 0b11;
    if (l3[pte_index] & 1)
    {
        LOG("[MMU warning]: Section already mapped (%x).\n", va);
        return;
    }

    l3[pte_index] = (pa & PAGE_MASK) | attr;
}

void mmu_map_block(unsigned long *pgd, unsigned long va, unsigned long pa, unsigned long index, enum MMU_Flags flags)
{
    unsigned long l1_index = (va >> 39) & 0x1FF;
    unsigned long l2_index = (va >> 30) & 0x1FF;
    unsigned long l3_index = (va >> 21) & 0x1FF;

    if (!(pgd[l1_index] & 1))
    {
        unsigned long *l1 = malloc(PAGE_SIZE);
        memset(l1, 0, PAGE_SIZE);
        pgd[l1_index] = (VIRT_TO_PHYS((unsigned long)l1) & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l1 = (unsigned long *)PHYS_TO_VIRT((pgd[l1_index] & PAGE_MASK));

    if (!(l1[l2_index] & 1))
    {
        unsigned long *l2 = malloc(PAGE_SIZE);
        memset(l2, 0, PAGE_SIZE);
        l1[l2_index] = (VIRT_TO_PHYS((unsigned long)l2) & PAGE_MASK) | PD_TABLE;
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

unsigned long memory_i;

void mmu_init()
{
    // retain the amount of memory allocated in total
    memory_i = 0;
    unsigned long *pgd = temp_malloc(memory_i);
    memory_i++;
    unsigned long *high_pgd = temp_malloc(memory_i);
    memory_i++;

    // make 2 maps: a lower half and a higher half
    for (unsigned long addr = 0; addr <= GRANULE_1GB * 4; addr += GRANULE_2MB)
    {
        if (addr < DEVICE_START)
        {
            memory_i = mmu_map_temp_block(high_pgd, HIGH_VA + addr, addr, MAIR_IDX_NORMAL, true, memory_i);
            memory_i = mmu_map_temp_block(pgd, addr, addr, MAIR_IDX_NORMAL, true, memory_i);
        }
        else
        {
            memory_i = mmu_map_temp_block(pgd, addr, addr, MAIR_IDX_DEVICE, true, memory_i);
            memory_i = mmu_map_temp_block(high_pgd, HIGH_VA + addr, addr, MAIR_IDX_DEVICE, true, memory_i);
        }
    }

    // init the regs with both halves
    mmu_init_regs((unsigned long)pgd, (unsigned long)high_pgd);
}

extern void kmain();

void finish_higher()
{
    mm_init();

    unsigned long *pgd = malloc(PAGE_SIZE);

    // redo ttbr1 with the new persistent memory allocation
    // rather than the temporary one
    for (unsigned long addr = HIGH_VA; addr <= HIGH_VA + GRANULE_1GB * 4; addr += GRANULE_2MB)
    {
        if (addr < HIGH_VA + DEVICE_START)
            mmu_map_block(pgd, addr, addr - HIGH_VA, MAIR_IDX_NORMAL, MMU_NORW);
        else
            mmu_map_block(pgd, addr, addr - HIGH_VA, MAIR_IDX_DEVICE, MMU_NORW);
    }

    // make ttbr0 invalid, use only ttbr1
    refresh_ttbr(pgd);

    kmain();
}