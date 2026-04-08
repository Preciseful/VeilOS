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
- Users and groups.

## Building
1. Clone the repository.
2. Run: `./setup.sh`
    - Make sure an SD card for raspberry pi is prepared.
3. Run: `make`