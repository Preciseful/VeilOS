#pragma once

#define BOOT_SIGNATURE 0xAA55

typedef struct __attribute__((packed)) ChsAddress
{
    unsigned char head;
    unsigned char sector : 6;
    unsigned char cylinder_hi : 2;
    unsigned char cylinder_lo;
} ChsAddress;

typedef struct __attribute__((packed)) PartitionEntry
{
    unsigned char status;
    struct ChsAddress first_sector;
    unsigned char type;
    struct ChsAddress last_sector;
    unsigned int first_lba_sector;
    unsigned int num_sectors;
} PartitionEntry;

typedef struct __attribute__((packed)) MasterBootRecord
{
    unsigned char bootCode[0x1BE];
    struct PartitionEntry partitions[4];
    unsigned short bootSignature;
} MasterBootRecord;