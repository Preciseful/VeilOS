#include <std/stdlib.h>
#include <svc.h>

void *malloc(unsigned long size)
{
    return (void *)svc_call(size, 0, 0, 0, 0, 0, 0, 0, SYS_MALLOC);
}

void free(void *data)
{
    svc_call((unsigned long)data, 0, 0, 0, 0, 0, 0, 0, SYS_FREE);
}