#include <interface/partition.h>
#include <drivers/emmc.h>
#include <lib/printf.h>

Partition *PartitionsInit()
{
    MasterBootRecord mbr;
    SeekInEMMC(0);
    ReadFromEMMC((unsigned char *)&mbr, sizeof(mbr));

    if (mbr.bootSignature != BOOT_SIGNATURE)
    {
        LOG("Bad MBR signature.\n");
        return 0;
    }

    unsigned long count = 0;
    for (unsigned long i = 0; mbr.partitions[i].type != 0; i++)
        count++;

    Partition *partitions = malloc(count * sizeof(Partition));

    for (unsigned long i = 0; mbr.partitions[i].type != 0; i++)
    {
        char *name = malloc(PAGE_SIZE);
        SPrintf(name, "SD#%lu", i);
        unsigned long sector = mbr.partitions[i].first_lba_sector * 512;

        Partition partition;
        partition.name = name;
        partition.mbr = mbr;
        partition.offset = sector;

        partitions[i] = partition;
    }

    return partitions;
}