#pragma once

#include <fs/mbr.h>

typedef struct Partition
{
    char *name;
    unsigned long offset;
    MasterBootRecord mbr;
} Partition;

Partition *PartitionsInit();