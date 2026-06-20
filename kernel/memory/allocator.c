/**
 * @author Developful
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <memory/allocator.h>
#include <lib/printf.h>
#include <memory/mmu.h>
#include <scheduler/task.h>
#include <scheduler/scheduler.h>
#include <lib/string.h>
#include <lib/panic.h>
#include <limits.h>
#include <drivers/gic.h>
#include <interface/dtb.h>

unsigned long *pgd;

extern char bss_begin[];
extern char bss_end[];

void MMInit(void *dtb, Page *memmap, unsigned long memsize, unsigned long lp)
{
    PMMInit(dtb, memsize, (Page *)memmap, lp);
    pgd = (unsigned long *)(HIGH_VA + LOW_MEMORY);
}

bool check_memory(unsigned long index, unsigned int size)
{
    for (unsigned long i = 0; i < size / PAGE_SIZE && index + i < GetAmountOfPages(); i++)
    {
        if (GetPageAtIndex(index + i)->flags != PAGE_FREE)
            return false;
    }

    return true;
}

void *get_free_page(unsigned int size)
{
    for (int i = 0; i < GetAmountOfPages(); i++)
    {
        if (check_memory(i, size))
        {
            unsigned long pages = size / PAGE_SIZE;
            for (unsigned long j = 0; j < pages; j++)
            {
                PhysicalAddr caddr = (i + j) * PAGE_SIZE;
                MapTablePage(pgd, HIGH_VA + caddr, caddr, MAIR_IDX_NORMAL, MMU_NORW);

                if (j < pages - 1)
                    GetPageAtIndex(i + j)->flags = PAGE_OCCUPIED;
                else
                    GetPageAtIndex(i + j)->flags = PAGE_HEADER;
            }

            unsigned long adr = HIGH_VA + i * PAGE_SIZE;
            return (void *)adr;
        }
    }

    return 0;
}

MHeader *get_header(unsigned long data_index)
{
    for (unsigned long i = data_index; i < GetAmountOfPages(); i++)
    {
        if (GetPageAtIndex(i)->flags != PAGE_HEADER)
            continue;

        return (MHeader *)(HIGH_VA + i * PAGE_SIZE);
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

    return header->data;
}

unsigned int free(void *data)
{
    unsigned long index = ((unsigned long)data - HIGH_VA) / PAGE_SIZE;
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

        GetPageAtIndex(index + i)->flags = false;
    }

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

unsigned int MemorySize(void *data)
{
    unsigned long index = ((unsigned long)data - HIGH_VA) / PAGE_SIZE;
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