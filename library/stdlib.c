#include "stdlib.h"
#include "svc.h"

void *malloc(unsigned int size)
{
    return (void *)svc_call(size, 0, 0, 0, 0, 0, 0, 0, 1);
}

void free(void *data)
{
    svc_call((unsigned long)data, 0, 0, 0, 0, 0, 0, 0, 2);
}