#ifndef MBR_HPP
#define MBR_HPP

#define BOOT_SIGNATURE 0xAA55

namespace veil::fs
{
    struct chs_address
    {
        unsigned char head;
        unsigned char sector : 6;
        unsigned char cylinder_hi : 2;
        unsigned char cylinder_lo;
    } __attribute__((packed));

    struct partition_entry
    {
        unsigned char status;
        chs_address first_sector;
        unsigned char type;
        chs_address last_sector;
        unsigned int first_lba_sector;
        unsigned int num_sectors;
    } __attribute__((packed));

    struct master_boot_record
    {
        unsigned char bootCode[0x1BE];
        partition_entry partitions[4];
        unsigned short bootSignature;
    } __attribute__((packed));
}

#endif