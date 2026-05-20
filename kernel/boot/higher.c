#include <memory/pmm.h>
#include <memory/memory.h>
#include <drivers/gic.h>
#include <boot/higher.h>
#include <drivers/uart.h>
#include <interface/dtb.h>
#include <lib/string.h>

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_ACCESS (1 << 10)

#define PAGE_TABLE_ENTRIES 512
#define PAGE_MASK 0xFFFFFFFFF000ULL

unsigned long last_page;

void *temp_malloc()
{
    last_page += GRANULE_4KB;
    return (void *)(last_page - GRANULE_4KB);
}

void mmu_map_temp_page(unsigned long *pgd, unsigned long va, unsigned long pa, unsigned long index, bool kernel)
{
    unsigned long l1_index = (va >> 39) & 0x1FF;
    unsigned long l2_index = (va >> 30) & 0x1FF;
    unsigned long l3_index = (va >> 21) & 0x1FF;
    unsigned long pte_index = (va >> 12) & 0x1FF;

    if (!(pgd[l1_index] & 1))
    {
        unsigned long *l1 = temp_malloc();
        memset(l1, 0, PAGE_SIZE);
        pgd[l1_index] = ((unsigned long)l1 & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l1 = (unsigned long *)(pgd[l1_index] & PAGE_MASK);

    if (!(l1[l2_index] & 1))
    {
        unsigned long *l2 = temp_malloc();
        memset(l2, 0, PAGE_SIZE);
        l1[l2_index] = ((unsigned long)l2 & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l2 = (unsigned long *)(l1[l2_index] & PAGE_MASK);

    if (!(l2[l3_index] & 1))
    {
        unsigned long *l3 = temp_malloc();
        memset(l3, 0, PAGE_SIZE);
        l2[l3_index] = ((unsigned long)l3 & PAGE_MASK) | PD_TABLE;
    }

    unsigned long *l3 = (unsigned long *)(l2[l3_index] & PAGE_MASK);

    unsigned char perm = kernel ? 0b00 : 0b01;
    unsigned long attr = ((unsigned long)1 << 54) | ((unsigned long)0 << 53) | PD_ACCESS | (0b11 << 8) | (perm << 6) | (index << 2) | 0b11;

    l3[pte_index] = (pa & 0xFFFFFFFFF000ULL) | attr;
}

struct
{
    Page *memmap;
    unsigned long memsize;
    void *dtb;
} mmdata;

extern char __kernel_start[];
extern char __kernel_code[];
extern char __kernel_end[];

void JumpToHigher(void *dtb)
{
    unsigned int *data;
    unsigned int len = ParseDTB(dtb, "/memory@0/reg", (void **)&data);
    unsigned long memsize = 0;

    for (int i = 0; i < len / 12; i++)
    {
        unsigned int *entry = data + i * 3;
        unsigned long start_address = ((unsigned long)__builtin_bswap32(entry[0]) << 32) | __builtin_bswap32(entry[1]);
        unsigned int size = __builtin_bswap32(entry[2]);
        unsigned long end_address = start_address + size;

        if (end_address > memsize)
            memsize = end_address;
    }

    last_page = LOW_MEMORY;
    unsigned long *high_pgd = temp_malloc();

    for (unsigned long addr = (unsigned long)__kernel_start - HIGH_VA; addr <= LOW_MEMORY; addr += GRANULE_4KB)
        mmu_map_temp_page(high_pgd, HIGH_VA + addr, addr, MAIR_IDX_NORMAL, true);

    for (unsigned long addr = (unsigned long)dtb; addr < (unsigned long)dtb + GetDTBSize(dtb); addr += GRANULE_4KB)
        mmu_map_temp_page(high_pgd, HIGH_VA + addr, addr, MAIR_IDX_NORMAL, true);

    for (unsigned long addr = PERIPHERAL_BASE - HIGH_VA; addr <= (PERIPHERAL_BASE - HIGH_VA) + GRANULE_2MB * 8; addr += GRANULE_4KB)
        mmu_map_temp_page(high_pgd, HIGH_VA + addr, addr, MAIR_IDX_DEVICE, true);

    for (unsigned long addr = GIC_BASE; addr <= GIC_BASE + GRANULE_4KB * 2; addr += GRANULE_4KB)
        mmu_map_temp_page(high_pgd, HIGH_VA + addr, addr, MAIR_IDX_DEVICE, true);

    mmu_map_temp_page(high_pgd, HIGH_VA, 0, MAIR_IDX_NORMAL, true);

    mmdata.dtb = HIGH_VA + dtb;
    mmdata.memmap = (Page *)(HIGH_VA + last_page);
    mmdata.memsize = memsize;
    for (unsigned long i = 0; i <= (memsize * sizeof(Page)) / (GRANULE_4KB * GRANULE_4KB); i++)
        temp_malloc();

    for (unsigned long addr = LOW_MEMORY; addr <= last_page; addr += GRANULE_4KB)
        mmu_map_temp_page(high_pgd, HIGH_VA + addr, addr, MAIR_IDX_DEVICE, true);

    unsigned long *pgd = temp_malloc();
    for (unsigned long addr = (unsigned long)__kernel_start - HIGH_VA; addr <= (unsigned long)__kernel_code - HIGH_VA; addr += GRANULE_4KB)
        mmu_map_temp_page(pgd, addr, addr, MAIR_IDX_NORMAL, true);

    jump_high((unsigned long)pgd, (unsigned long)high_pgd, last_page);
}

extern void kmain();

void finish_higher(unsigned long lp)
{
    UartInit();
    MMInit(mmdata.dtb, mmdata.memmap, mmdata.memsize, lp);

    refresh_ttbr(LOW_MEMORY);

    kmain();
}