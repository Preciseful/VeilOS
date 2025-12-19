#pragma once

#include <boot/base.h>
#include <stdbool.h>

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define HIGH_VA 0xFFFF000000000000ULL
#define VIRT_TO_PHYS(x) ((VirtualAddr)x - HIGH_VA)
#define PHYS_TO_VIRT(x) ((PhysicalAddr)x + HIGH_VA)

#define LOW_MEMORY (2 * SECTION_SIZE)
#define HIGH_MEMORY (PERIPHERAL_BASE - HIGH_VA)

#define PAGING_MEMORY (HIGH_MEMORY - LOW_MEMORY - 0x80000)
#define PAGING_PAGES (PAGING_MEMORY / PAGE_SIZE)

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9

#ifndef __ASSEMBLER__

#include <interface/syscall.h>

typedef struct MHeader
{
    unsigned long size;
    struct vheader *next;
    void *data;
} MHeader;

void MMInit();
void *malloc(unsigned int size);
unsigned int free(void *data);
unsigned int memory_size(void *data);
void memset(void *dest, int value, unsigned long size);
void memcpy(void *dst, const void *src, unsigned long size);
int memcmp(const void *m1, const void *m2, unsigned long n);
unsigned long get_memory_used();

SYSCALL_HANDLER(malloc);
SYSCALL_HANDLER(free);
SYSCALL_HANDLER(memory_size);

#endif