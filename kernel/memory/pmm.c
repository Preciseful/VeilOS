#include <memory/pmm.h>
#include <memory/memory.h>
#include <drivers/gic.h>
#include <lib/string.h>
#include <interface/dtb.h>

Page *mem_map;
unsigned long pages_count;

void PMMInit(void *dtb, unsigned long memsize, Page *allocated_memmap, unsigned long last_page)
{
    mem_map = (Page *)allocated_memmap;
    memset(mem_map, 1, memsize / PAGE_SIZE);

    unsigned int *data;
    unsigned int len = ParseDTB(dtb, "/memory@0/reg", (void **)&data);

    for (int i = 0; i < len / 12; i++)
    {
        unsigned int *entry = data + i * 3;
        unsigned long address = ((unsigned long)__builtin_bswap32(entry[0]) << 32) | __builtin_bswap32(entry[1]);
        unsigned int size = __builtin_bswap32(entry[2]);
        unsigned long end_address = address + size;

        for (unsigned long addr = address; addr < end_address; addr += PAGE_SIZE)
            mem_map[addr / PAGE_SIZE].flags = 0;
    }

    for (unsigned long addr = HIGH_VA + LOW_MEMORY; addr <= HIGH_VA + last_page; addr += PAGE_SIZE)
    {
        unsigned long i = ((unsigned long)addr - HIGH_VA) / PAGE_SIZE;
        mem_map[i].flags = PAGE_RESERVED;
    }

    for (unsigned long addr = HIGH_VA; addr <= HIGH_VA + LOW_MEMORY; addr += PAGE_SIZE)
    {
        unsigned long i = ((unsigned long)addr - HIGH_VA) / PAGE_SIZE;
        mem_map[i].flags = PAGE_RESERVED;
    }

    for (unsigned long addr = PERIPHERAL_BASE; addr <= PERIPHERAL_BASE + (PAGE_SIZE * PAGE_SIZE); addr += PAGE_SIZE)
    {
        unsigned long i = ((unsigned long)addr - HIGH_VA) / PAGE_SIZE;
        mem_map[i].flags = PAGE_RESERVED;
    }

    for (unsigned long addr = GIC_BASE; addr <= GIC_BASE + PAGE_SIZE * 2; addr += PAGE_SIZE)
    {
        unsigned long i = ((unsigned long)addr - HIGH_VA) / PAGE_SIZE;
        mem_map[i].flags = PAGE_RESERVED;
    }

    pages_count = memsize / PAGE_SIZE;
}

Page *GetPageAtAddress(unsigned long address)
{
    if (address - HIGH_VA / PAGE_SIZE >= GetAmountOfPages())
        return 0;

    return &mem_map[(address - HIGH_VA) / PAGE_SIZE];
}

Page *GetPageAtIndex(unsigned long index)
{
    if (index >= GetAmountOfPages())
        return 0;

    return &mem_map[index];
}

unsigned long GetAmountOfPages()
{
    return pages_count;
}

PhysicalAddr GetPhysicalPage()
{
    for (int i = 0; i < pages_count; i++)
    {
        if (mem_map[i].flags == PAGE_FREE)
        {
            mem_map[i].flags = PAGE_OCCUPIED;
            return i * PAGE_SIZE;
        }
    }

    return 0;
}