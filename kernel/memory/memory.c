/**
 * @author Developful
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <memory/memory.h>
#include <lib/printf.h>
#include <memory/mmu.h>
#include <scheduler/task.h>
#include <scheduler/scheduler.h>
#include <lib/string.h>
#include <lib/panic.h>
#include <limits.h>
#include <drivers/gic.h>

__attribute__((section(".mmmap"))) static unsigned char mem_map[PAGING_PAGES];

unsigned long used_memory = 0;
unsigned long total_memory;
unsigned long *pgd;

extern char bss_begin[];
extern char bss_end[];

void MMInit(unsigned long lp)
{
    memset(mem_map, 0, PAGING_PAGES);

    total_memory = PAGING_PAGES * PAGE_SIZE;

    unsigned long lpi = (lp - LOW_MEMORY) / PAGE_SIZE;
    for (unsigned long i = 0; i <= lpi; i++)
        mem_map[i] = 1;

    for (unsigned long addr = PERIPHERAL_BASE; addr <= PERIPHERAL_BASE + GRANULE_2MB * 8; addr += GRANULE_4KB)
    {
        total_memory -= PAGE_SIZE;
        unsigned long i = ((unsigned long)addr - HIGH_VA - LOW_MEMORY) / PAGE_SIZE;
        mem_map[i] = 1;
    }

    for (unsigned long addr = GIC_BASE; addr <= GIC_BASE + GRANULE_4KB * 2; addr += GRANULE_4KB)
    {
        total_memory -= PAGE_SIZE;
        unsigned long i = ((unsigned long)addr - HIGH_VA - LOW_MEMORY) / PAGE_SIZE;
        mem_map[i] = 1;
    }

    pgd = (unsigned long *)(HIGH_VA + LOW_MEMORY);
    used_memory += lpi * PAGE_SIZE;
}

bool check_memory(unsigned long index, unsigned int size)
{
    for (unsigned long i = 0; i < size / PAGE_SIZE; i++)
    {
        if (mem_map[index + i])
            return false;
    }

    return true;
}

PhysicalAddr GetPhysicalPage()
{
    for (int i = 0; i < PAGING_PAGES; i++)
    {
        if (mem_map[i] == 0)
        {
            mem_map[i] = 1;
            return LOW_MEMORY + i * PAGE_SIZE;
        }
    }

    return 0;
}

void *get_free_page(unsigned int size)
{
    for (int i = 0; i < PAGING_PAGES; i++)
    {
        if (check_memory(i, size))
        {
            unsigned long pages = size / PAGE_SIZE;
            for (unsigned long j = 0; j < pages; j++)
            {
                PhysicalAddr caddr = LOW_MEMORY + (i + j) * PAGE_SIZE;
                MapTablePage(pgd, HIGH_VA + caddr, caddr, MAIR_IDX_NORMAL, MMU_NORW);

                if (j < pages - 1)
                    mem_map[i + j] = true;
                else
                    mem_map[i + j] = 2;
            }

            unsigned long adr = HIGH_VA + LOW_MEMORY + i * PAGE_SIZE;
            return (void *)adr;
        }
    }

    return 0;
}

MHeader *get_header(unsigned long data_index)
{
    for (unsigned long i = data_index; i < PAGING_PAGES; i++)
    {
        if (mem_map[i] != 2)
            continue;

        return (MHeader *)(HIGH_VA + LOW_MEMORY + i * PAGE_SIZE);
    }

    return 0;
}

void *malloc(unsigned int size)
{
    if (size == 0)
        return 0;

    unsigned int initial_size = size;
    size = ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

    void *page = get_free_page(PAGE_SIZE + size);
    MHeader *header = (MHeader *)(page + size);

    header->data = page;
    header->size = size;
    header->initial_size = initial_size;

    used_memory += header->size;
    return header->data;
}

unsigned int free(void *data)
{
    unsigned long index = ((unsigned long)data - HIGH_VA - LOW_MEMORY) / PAGE_SIZE;
    MHeader *header = get_header(index);

    if (header == 0)
        panic("Free: Memory header could not be found.");

    unsigned long initial_size = header->initial_size;
    unsigned long size = header->size;

    for (unsigned long i = 0; i < (size / PAGE_SIZE) + 1; i++)
    {
        VirtualAddr va = (VirtualAddr)data + i * PAGE_SIZE;

        UnmapTablePage(pgd, va);
        reset_va(va >> 12);

        mem_map[index + i] = false;
    }

    used_memory -= size;
    return initial_size;
}

void *realloc(void *address, unsigned int size)
{
    if (size <= MemorySize(address))
        panic("Cannot reallocate with a smaller size or equal than initial.");

    void *new_address = malloc(size);
    memcpy(new_address, address, size);
    free(address);

    return new_address;
}

unsigned long GetMemoryUsed()
{
    return used_memory;
}

unsigned long GetTotalMemory()
{
    return total_memory;
}

unsigned int MemorySize(void *data)
{
    unsigned long index = ((unsigned long)data - HIGH_VA - LOW_MEMORY) / PAGE_SIZE;
    MHeader *header = get_header(index);

    if (header == 0)
        panic("MemorySize: Memory header could not be found.");
    return header->initial_size;
}

SYSCALL_HANDLER(malloc)
{
    if (sp->x0 >= UINT_MAX)
        return 0;

    unsigned int size = sp->x0;

    Task *task = GetRunningTask();
    VirtualAddr va = GetTaskValidVA(task, size);
    PhysicalAddr pa = VIRT_TO_PHYS(malloc(size));

    MapTaskPage(GetRunningTask(), va, pa, size, MMU_RWRW);

    return va;
}

SYSCALL_HANDLER(free)
{
    PhysicalAddr pa = GetPagePA(GetRunningTask(), sp->x0);
    if (pa == 0)
        return 0;

    unsigned long len = free((void *)PHYS_TO_VIRT(pa));
    UnmapTaskPage(GetRunningTask(), sp->x0, len);

    return len;
}

SYSCALL_HANDLER(memory_size)
{
    PhysicalAddr pa = GetPagePA(GetRunningTask(), sp->x0);
    if (pa == 0)
        return 0;

    return MemorySize((void *)PHYS_TO_VIRT(pa));
}