#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include "svc.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

inline void *malloc(unsigned int size)
{
    return (void *)svc0(1, size);
}

inline void free(void *data)
{
    svc0(2, (unsigned long)data);
}

inline void *realloc(void *buf, unsigned int oldsize, unsigned int newsize)
{
    void *newloc = (void *)malloc(newsize);
    for (int i = 0; i < oldsize; i++)
        ((char *)newloc)[i] = ((char *)buf)[i];

    free(buf);
    return newloc;
}

inline void *operator new(size_t size)
{
    return malloc(size);
}

inline void *operator new[](size_t size)
{
    return malloc(size);
}

inline void operator delete(void *p)
{
    free(p);
}

inline void operator delete(void *ptr, unsigned long extra)
{
    free(ptr);
}

inline void operator delete[](void *p)
{
    free(p);
}

inline void operator delete[](void *p, unsigned long extra)
{
    free(p);
}

inline bool fork()
{
    return svc(4);
}

#pragma GCC diagnostic pop

#endif
