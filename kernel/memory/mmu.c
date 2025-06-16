#include <memory/mmu.h>
#include <boot/base.h>
#include <memory/memory.h>

#define TD_VALID (1 << 0)
#define TD_BLOCK (0 << 1)
#define TD_TABLE (1 << 1)
#define TD_ACCESS (1 << 10)
#define TD_KERNEL_PERMS (1L << 54)
#define TD_INNER_SHARABLE (3 << 8)

#define TD_KERNEL_TABLE_FLAGS (TD_TABLE | TD_VALID)
#define TD_KERNEL_BLOCK_FLAGS (TD_ACCESS | TD_INNER_SHARABLE | TD_KERNEL_PERMS | (MATTR_NORMAL_NC_INDEX << 2) | TD_BLOCK | TD_VALID)
#define TD_DEVICE_BLOCK_FLAGS (TD_ACCESS | TD_INNER_SHARABLE | TD_KERNEL_PERMS | (MATTR_DEVICE_nGnRnE_INDEX << 2) | TD_BLOCK | TD_VALID)

#define MATTR_DEVICE_nGnRnE 0x0
#define MATTR_NORMAL_NC 0x44
#define MATTR_DEVICE_nGnRnE_INDEX 0
#define MATTR_NORMAL_NC_INDEX 1
#define MAIR_EL1_VAL ((MATTR_NORMAL_NC << (8 * MATTR_NORMAL_NC_INDEX)) | MATTR_DEVICE_nGnRnE << (8 * MATTR_DEVICE_nGnRnE_INDEX))

#define ID_MAP_PAGES 6
#define PGD_SIZE (ID_MAP_PAGES * PAGE_SIZE)
#define TABLE_ENTRIES 512
#define PGD_SHIFT (PAGE_SHIFT + 3 * TABLE_SHIFT)
#define PUD_SHIFT (PAGE_SHIFT + 2 * TABLE_SHIFT)
#define PMD_SHIFT (PAGE_SHIFT + TABLE_SHIFT)
#define PUD_ENTRY_MAP_SIZE (1 << PUD_SHIFT)

#define BLOCK_SIZE 0x40000000

extern unsigned long pgd_address();

void make_entry(unsigned long table, unsigned long entry, unsigned long va, unsigned long shift, unsigned long flags)
{
    unsigned long index = va >> shift;
    index &= (TABLE_ENTRIES - 1);
    unsigned long desc = entry | flags;
    *((unsigned long *)(table + (index << 3))) = desc;
}

void make_block(unsigned long pmd, unsigned long vstart, unsigned long vend, unsigned long pa, unsigned long shift)
{
    vstart >>= shift;
    vend >>= shift;
    vend--;

    vstart &= (TABLE_ENTRIES - 1);
    vend &= (TABLE_ENTRIES - 1);

    pa >>= shift;
    pa <<= shift;

    do
    {
        unsigned long temp_pa = pa;
        if (pa >= DEVICE_START)
            temp_pa |= TD_DEVICE_BLOCK_FLAGS;
        else
            temp_pa |= TD_KERNEL_BLOCK_FLAGS;

        unsigned long *table_ptr = (unsigned long *)(pmd + (vstart << 3));
        *table_ptr = temp_pa;

        pa += (1 << shift);
        vstart++;
    } while (vstart <= vend);
}

void mmu_init()
{
    unsigned long pgd = pgd_address();
    memset((void *)pgd, 0, PGD_SIZE);

    unsigned long pud = pgd + PAGE_SIZE;
    unsigned long pmd = pud + PAGE_SIZE;

    make_entry(pgd, pud, 0, PGD_SHIFT, TD_KERNEL_TABLE_FLAGS);

    for (unsigned long i = 0; i < 4; i++)
    {
        unsigned long va = i * BLOCK_SIZE;
        make_entry(pud, pmd, va, PUD_SHIFT, TD_KERNEL_TABLE_FLAGS);
        make_block(pmd, va, va + BLOCK_SIZE, va, PMD_SHIFT);
        pmd += PAGE_SIZE;
    }
}