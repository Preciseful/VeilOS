#include <mm.h>
#include <lib/printf.h>

static struct vheader headers[PAGING_PAGES];
static unsigned short mem_map[PAGING_PAGES] = {0};

unsigned long get_free_page()
{
    for (int i = 0; i < PAGING_PAGES; i++)
    {
        if (mem_map[i] == 0)
        {
            mem_map[i] = 1;
            unsigned long adr = LOW_MEMORY + i * PAGE_SIZE;
            // printf("Found free page: %lu\n", adr);
            return adr;
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

unsigned long valloc(unsigned int size)
{
    int headerI = get_free_header();
    if (headerI == -1)
    {
        ERROR("No available headers.");
        return 0;
    }

    headers[headerI].in_use = true;
    headers[headerI].extension = false;

    size -= PAGE_SIZE_BYTES > size ? size : PAGE_SIZE_BYTES;
    headers[headerI].data = get_free_page();

    for (int i = 1; size; i++)
    {
        headers[headerI + i].in_use = true;
        headers[headerI].extension = true;
        get_free_page();
        size -= PAGE_SIZE_BYTES > size ? size : PAGE_SIZE_BYTES;
    }

    return headers[headerI].data;
}

void vfree(void *data)
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

void memcpy(void *dst, const void *src, unsigned long size)
{
    for (int i = 0; i < size; i++)
        ((char *)dst)[i] = ((char *)src)[i];
}

void memory_init()
{
}