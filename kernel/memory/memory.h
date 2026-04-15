/**
 * @file
 * @author Developful
 * @brief Memory allocation interface.
 * @date 2026-04-10
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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

#include <system/syscall.h>

typedef unsigned long VirtualAddr;
typedef unsigned long PhysicalAddr;

typedef struct MHeader
{
    unsigned int size;
    void *data;
    unsigned int initial_size;
} MHeader;

/**
 * @brief Initialize the memory manager.
 */
void MMInit(unsigned long lp);

PhysicalAddr GetPhysicalPage();

void *malloc(unsigned int size);
void *realloc(void *address, unsigned int size);
unsigned int free(void *data);

/**
 * @brief Get the amount of memory allocated to an address.
 *
 * @param data The address.
 * @return The amount of memory allocated.
 */
unsigned int MemorySize(void *data);

/**
 * @return The amount of memory so far used.
 */
unsigned long GetMemoryUsed();

SYSCALL_HANDLER(malloc);
SYSCALL_HANDLER(free);
SYSCALL_HANDLER(memory_size);

#endif