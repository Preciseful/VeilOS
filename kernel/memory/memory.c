#include <memory/memory.h>
#include <lib/printf.h>

static mheader_t default_header = {0, 0, 0, 0, 0, 0};

unsigned long get_page(unsigned long i)
{
    unsigned long adr = LOW_MEMORY + i * PAGE_SIZE;
    return adr;
}

mheader_t *get_free_header()
{
    if (default_header.next == 0)
    {
        default_header.next = (mheader_t *)get_page(0);
        default_header.next->next = 0;
        default_header.next->pos = 0;
        return default_header.next;
    }
    else
    {
        mheader_t *last = &default_header;
        while (last->next)
        {
            if (!last->next->in_use)
                break;

            last = last->next;
        }

        if (last->next == 0)
        {
            last->next = (mheader_t *)get_page(last->pos + 1);
            last->next->next = 0;
            last->next->pos = last->pos + 1;
        }

        return last->next;
    }

    return 0;
}

void *malloc(unsigned int size)
{
    mheader_t *header = get_free_header();
    if (header == 0)
    {
        printf("No available headers.");
        return 0;
    }

    header->in_use = true;
    header->extension = false;
    header->data = get_page(header->pos);
    size -= PAGE_SIZE_BYTES > size ? size : PAGE_SIZE_BYTES;

    while (size)
    {
        mheader_t *subheader = get_free_header();
        size -= PAGE_SIZE_BYTES > size ? size : PAGE_SIZE_BYTES;

        subheader->in_use = true;
        subheader->extension = true;
    }

    return (void *)header->data;
}

void free(void *data)
{
    unsigned long pos = ((unsigned long)data - LOW_MEMORY) / PAGE_SIZE;
    mheader_t *header = &default_header;
    for (unsigned long i = 0; i < pos + 1; i++)
        header = header->next;

    if (header->pos != pos)
    {
        printf("BAD HEADER !!");
        return;
    }

    header->in_use = false;

    while (header->next)
    {
        header = header->next;
        if (header->extension == true)
            header->in_use = false;
    }
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