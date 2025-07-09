#pragma once

#include <boot/base.h>
#include <stdbool.h>

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGE_SIZE_BYTES (PAGE_SIZE / 8)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define LOW_MEMORY (2 * SECTION_SIZE)
#define HIGH_MEMORY PERIPHERAL_BASE

#define PAGING_MEMORY (HIGH_MEMORY - LOW_MEMORY - 0x80000)
#define PAGING_PAGES (PAGING_MEMORY / PAGE_SIZE)

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9

#define HIGH_VA 0xFFFF000000000000

#ifndef __ASSEMBLER__

typedef struct mheader
{
    unsigned long size;
    bool in_use;
    bool extension;
    struct vheader *next;
    void *data;
} mheader_t;

void mm_init();
void *malloc(unsigned int size);
void free(void *data);
void memset(void *dest, int value, unsigned long size);
void memcpy(void *dst, const void *src, unsigned long size);
int memcmp(const void *m1, const void *m2, unsigned long n);

#endif