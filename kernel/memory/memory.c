#include <memory/memory.h>
#include <lib/printf.h>

#define HEADERS_BYTES (sizeof(mheader_t) * PAGING_PAGES)
#define HEADERS_PAGES ((HEADERS_BYTES + PAGE_SIZE - 1) / PAGE_SIZE)

__attribute__((section(".mmmap"))) static mheader_t headers[PAGING_PAGES];
__attribute__((section(".mmmap"))) static unsigned char mem_map[PAGING_PAGES];

void mm_init()
{
    printf("Zeroing the memory...\n");
    memset(mem_map, 0, PAGING_PAGES);
    memset(headers, 0, PAGING_PAGES);
    printf("Finished zeroing the memory!\n");
}

void *get_free_page()
{
    for (int i = 0; i < PAGING_PAGES; i++)
    {
        if (mem_map[i] == 0)
        {
            mem_map[i] = 1;
            unsigned long adr = LOW_MEMORY + i * PAGE_SIZE;
            return (void *)adr;
        }
    }

    return 0;
}

void free_page(unsigned long p)
{
    mem_map[(p - LOW_MEMORY) / PAGE_SIZE] = 0;
}

int get_free_header()
{
    for (int i = 0; i < PAGING_PAGES; i++)
    {
        if (!headers[i].in_use)
            return i;
    }

    return -1;
}

void *malloc(unsigned int size)
{
    int headerI = get_free_header();
    if (headerI == -1)
    {
        printf("No available headers.\n");
        return 0;
    }

    headers[headerI].in_use = true;
    headers[headerI].extension = false;

    headers[headerI].data = get_free_page();
    size -= PAGE_SIZE_BYTES > size ? size : PAGE_SIZE_BYTES;

    for (int i = 1; size; i++)
    {
        headers[headerI + i].in_use = true;
        headers[headerI].extension = true;
        get_free_page();
        size -= PAGE_SIZE_BYTES > size ? size : PAGE_SIZE_BYTES;
    }

    return headers[headerI].data;
}

void free(void *data)
{
    unsigned long adr = ((unsigned long)data - LOW_MEMORY) / PAGE_SIZE;
    headers[adr].in_use = false;

    for (int i = adr; i < PAGING_PAGES; i++)
    {
        if (headers[i].extension)
        {
            headers[i].in_use = false;
            headers[i].extension = false;
        }

        else
            break;
    }

    mem_map[adr] = 0;
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

int memcmp(const void *m1, const void *m2, unsigned long n)
{
    const unsigned char *s1 = m1, *s2 = m2;
    for (unsigned long i = 0; i < n && *s1 == *s2; i++)
    {
        ++s1;
        ++s2;
    }

    return (*s1 > *s2) - (*s2 > *s1);
}