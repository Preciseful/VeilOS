#pragma once

#include <fs/mbr.h>

typedef struct Partition
{
    char *name;
    unsigned long offset;
    PartitionEntry backing_data;
    MasterBootRecord mbr;
} Partition;

Partition *PartitionsInit();