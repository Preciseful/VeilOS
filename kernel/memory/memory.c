#include <memory/memory.h>
#include <lib/printf.h>
#include <memory/mmu.h>
#include <scheduler/task.h>
#include <scheduler/scheduler.h>
#include <lib/string.h>
#include <lib/panic.h>
#include <limits.h>

__attribute__((section(".mmmap"))) static MHeader headers[PAGING_PAGES];
__attribute__((section(".mmmap"))) static unsigned char mem_map[PAGING_PAGES];

unsigned long used_memory = 0;

extern char bss_begin[];
extern char bss_end[];

void MMInit()
{
    // LOG("Zeroing the memory...\n");
    memset(mem_map, 0, PAGING_PAGES);
    memset(headers, 0, PAGING_PAGES);
    // LOG("Finished zeroing the memory!\n");
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

void *get_free_page(unsigned int size)
{
    for (int i = 0; i < PAGING_PAGES; i++)
    {
        if (check_memory(i, size))
        {
            for (unsigned long j = 0; j < size / PAGE_SIZE; j++)
                mem_map[i + j] = 1;

            unsigned long adr = HIGH_VA + LOW_MEMORY + i * PAGE_SIZE;
            return (void *)adr;
        }
    }

    return 0;
}

void *malloc(unsigned int size)
{
    if (size == 0)
        return 0;

    size = ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

    void *page = get_free_page(size);
    unsigned long index = ((unsigned long)page - LOW_MEMORY - HIGH_VA) / PAGE_SIZE;

    headers[index].data = page;
    headers[index].size = size;

    used_memory += headers[index].size;
    return headers[index].data;
}

unsigned int free(void *data)
{
    unsigned long index = ((unsigned long)data - HIGH_VA - LOW_MEMORY) / PAGE_SIZE;
    for (unsigned long i = 0; i < headers[index].size / PAGE_SIZE; i++)
        mem_map[index + i] = 0;

    used_memory -= headers[index].size;
    return headers[index].size;
}

void *realloc(void *address, unsigned int size)
{
    if (size < memory_size(address))
        panic("Cannot reallocate with a smaller size than initial.");

    void *new_address = malloc(size);
    memcpy(new_address, address, size);
    free(address);

    return new_address;
}

unsigned long get_memory_used()
{
    return used_memory;
}

unsigned int memory_size(void *data)
{
    unsigned long index = ((unsigned long)data - HIGH_VA - LOW_MEMORY) / PAGE_SIZE;
    return headers[index].size;
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
    if (!GetPagePA(GetRunningTask(), sp->x0))
        return 0;

    return memory_size((void *)sp->x0);
}