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

void PMMInit(void *dtb, unsigned long memsize, Page *allocated_memmap, unsigned long last_page);
Page *GetPageAtAddress(unsigned long address);
Page *GetPageAtIndex(unsigned long index);
unsigned long GetAmountOfPages();
PhysicalAddr GetPhysicalPage();