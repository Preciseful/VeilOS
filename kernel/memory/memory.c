#include <memory/memory.h>
#include <lib/printf.h>
#include <memory/mmu.h>
#include <scheduler/task.h>
#include <scheduler/scheduler.h>

__attribute__((section(".mmmap"))) static MHeader headers[PAGING_PAGES];
__attribute__((section(".mmmap"))) static unsigned char mem_map[PAGING_PAGES];

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
    size = ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

    void *page = get_free_page(size);
    unsigned long index = ((unsigned long)page - LOW_MEMORY - HIGH_VA) / PAGE_SIZE;

    headers[index].data = page;
    headers[index].size = size;

    return headers[index].data;
}

unsigned int free(void *data)
{
    unsigned long index = ((unsigned long)data - HIGH_VA - LOW_MEMORY) / PAGE_SIZE;
    for (unsigned long i = 0; i < headers[index].size / PAGE_SIZE; i++)
        mem_map[index + i] = 0;
    return headers[index].size;
}

unsigned int memory_size(void *data)
{
    unsigned long index = ((unsigned long)data - HIGH_VA - LOW_MEMORY) / PAGE_SIZE;
    return headers[index].size;
}

void memset(void *dest, int value, unsigned long size)
{
    unsigned char *ptr = (unsigned char *)dest;
    while (size--)
        *ptr++ = (unsigned char)value;

    return;
}

void memcpy(void *dst, const void *src, unsigned long size)
{
    for (int i = 0; i < size; i++)
        ((char *)dst)[i] = ((char *)src)[i];
}

int memcmp(const void *s1, const void *s2, unsigned long n)
{
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;

    for (unsigned long i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
            return p1[i] - p2[i];
    }

    return 0;
}

SYSCALL_HANDLER(malloc)
{
    unsigned long addr = VIRT_TO_PHYS(malloc(sp[0]));
    MapTaskPage(GetRunningTask(), addr, MMU_RWRW, addr, PAGE_SIZE);

    return addr;
}

SYSCALL_HANDLER(free)
{
    if (!TaskContainsVA(GetRunningTask(), sp[0]))
        return 0;

    unsigned long len = free((void *)PHYS_TO_VIRT(sp[0]));
    UnmapTaskPage(GetRunningTask(), sp[0], len);

    return len;
}

SYSCALL_HANDLER(memory_size)
{
    if (!TaskContainsVA(GetRunningTask(), sp[0]))
        return 0;

    return memory_size((void *)sp[0]);
}