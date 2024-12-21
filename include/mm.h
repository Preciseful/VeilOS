#ifndef MM_H
#define MM_H

#include <lib/base.h>
#include <stdbool.h>

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGE_SIZE_BYTES (PAGE_SIZE / 8)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define LOW_MEMORY (2 * SECTION_SIZE)
#define HIGH_MEMORY PERIPHERAL_BASE

#define PAGING_MEMORY (HIGH_MEMORY - LOW_MEMORY)
#define PAGING_PAGES (PAGING_MEMORY / PAGE_SIZE)

#ifndef __ASSEMBLER__

#define qalloc(x) (x *)valloc(sizeof(x))
#define qmemzero(ptr) memzero((unsigned long)ptr, sizeof(*ptr))

#ifdef __cplusplus
extern "C"
{
#endif

    struct vheader
    {
        unsigned long size;
        bool in_use;
        bool extension;
        struct vheader *next;
        unsigned long data;
    };

    extern void memzero(unsigned long src, unsigned int n);
    void memory_init();
    unsigned long get_free_page();
    void free_page(unsigned long p);
    unsigned long valloc(unsigned int size);
    void vfree(void *data);
    void memcpy(void *dst, const void *src, unsigned long size);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <stddef.h>

inline void *operator new(size_t size)
{
    return (void *)valloc(size);
}

inline void *operator new[](size_t size)
{
    return (void *)valloc(size);
}

inline void operator delete(void *p)
{
    vfree(p);
}

inline void operator delete(void *ptr, unsigned long extra)
{
    vfree(ptr);
}

inline void operator delete[](void *p)
{
    vfree(p);
}
#endif

#endif

#endif