/**
 * @author Developful
 * @date 2026-06-20
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory/mmu.h>

enum Page_Flags
{
    PAGE_FREE,
    PAGE_OCCUPIED,
    PAGE_HEADER,
    PAGE_RESERVED,
};

typedef struct Page
{
    unsigned char flags;
} Page;

/**
 * @brief Initialize the physical memory manager.
 *
 * @param dtb The machine's dtb data address.
 * @param memsize The total memory size of the machine.
 * @param allocated_memmap The early allocated memory map address.
 * @param last_page The last page early allocated.
 */
void PMMInit(void *dtb, unsigned long memsize, Page *allocated_memmap, unsigned long last_page);

/**
 * @brief Get the page object for a physical address.
 *
 * @param address The physical address.
 * @return The page object.
 */
Page *GetPageAtAddress(PhysicalAddr address);

/**
 * @brief Get the page object at an index.
 *
 * @param address The index.
 * @return The page object.
 */
Page *GetPageAtIndex(unsigned long index);

/**
 * @return Amount of total pages.
 */
unsigned long GetAmountOfPages();

/**
 * @return A free physical page.
 */
PhysicalAddr GetPhysicalPage();