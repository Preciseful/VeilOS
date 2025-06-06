#pragma once

#define BOOT_SIGNATURE 0xAA55

typedef struct __attribute__((packed)) chs_address
{
    unsigned char head;
    unsigned char sector : 6;
    unsigned char cylinder_hi : 2;
    unsigned char cylinder_lo;
} chs_address_t;

typedef struct __attribute__((packed)) partition_entry
{
    unsigned char status;
    struct chs_address first_sector;
    unsigned char type;
    struct chs_address last_sector;
    unsigned int first_lba_sector;
    unsigned int num_sectors;
} partition_entry_t;

typedef struct __attribute__((packed)) master_boot_record
{
    unsigned char bootCode[0x1BE];
    struct partition_entry partitions[4];
    unsigned short bootSignature;
} mbr_t;