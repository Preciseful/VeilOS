#include "stdlib.h"
#include "svc.h"

void *malloc(unsigned int size)
{
    return (void *)svc_call(size, 0, 0, 0, 0, 0, 0, 0, 1);
}