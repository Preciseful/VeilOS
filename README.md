# VeilOS

A simple, bare metal, raspberry pi 4B OS.

## Features
- Display available on UART and Framebuffer.
- VFS with FAT32 and Voidelle support.
- Userspace for EL0 and EL1 programs.
- SD card writing and reading.
- GIC support for scheduling/timer.
- Hardware RNG.
- ELF files support.
- Users for (terrible) security.

## Building
1. Clone the repository.
> [!CAUTION]
> `./setup.sh` will render previous data on the SD card unusable.
2. Run: `./setup.sh`.
    - Make sure an SD card for raspberry pi is prepared, this is to generate the right partitions and filesystems on them.
3. Run: `make`.
    - This generates the kernel8.img that raspberry pi requires to boot. It should automatically copy to the path given during setup.
    - It'll prompt you whether you wish to view the UART output or not.
4. You can now insert the card into the raspberry pi.

You can modify the variables within the `conf` file without re-doing the whole setup process.