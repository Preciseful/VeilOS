#!/bin/bash

read -p "Specify UART path to be used (default /dev/ttyUSB0): " TTYDEV
TTYDEV=${TTYDEV:-"/dev/ttyUSB0"}
echo "TTYDEV=$TTYDEV" > conf

RPI_DISK=""
while [[ $RPI_DISK == "" ]]; do
    echo "[ ! WARNING !] The disk will render previous data useless! Ensure the disk you type in is correct."
    read -p "Specify the Raspberry PI MicroSD disk to be used: " RPI_DISK

    if [[ $RPI_DISK != "" ]]; then
        answer=""

        while [[ $answer != [YyNn] ]]; do
            read -p "Is $RPI_DISK the right disk? (y/N): " answer
            answer=${answer:-"N"}

            if [[ $answer == [Nn] ]]; then
                RPI_DISK=""
            fi
        done
    fi
done

scripts/create_disk.sh $RPI_DISK > /dev/null

udisksctl mount -b "$RPI_DISK"1

RPI_PATH="/run/media/$USER/bootfs"
if ! [ -d $RPI_PATH ]; then
    read -p "Specify the Raspberry PI MicroSD path to be used: " RPI_PATH
fi

echo "RPI_PATH=$RPI_PATH" >> conf

if ! command -v aarch64-none-elf-gcc >/dev/null 2>&1; then
    GCC_PATH=""
    while ! [ -d "$GCC_PATH" ]; do
        read -p "C compiler for aarch64-none-elf not found. Install? (Y/n) " answer
        answer=${answer:-"Y"}

        if [[ $answer == [Yy] ]]; then
            wget https://developer.arm.com/-/media/Files/downloads/gnu/15.2.rel1/binrel/arm-gnu-toolchain-15.2.rel1-x86_64-aarch64-none-elf.tar.xz
            mkdir -p /home/$USER/.local/share/aarch64-none-elf/
            tar xf arm-gnu-toolchain-15.2.rel1-x86_64-aarch64-none-elf.tar.xz -C /home/$USER/.local/share/aarch64-none-elf/ --strip-components=1
            rm -rf arm-gnu-toolchain-15.2.rel1-x86_64-aarch64-none-elf.tar.xz
            GCC_PATH="/home/$USER/.local/share/aarch64-none-elf/aarch64-none-elf/"
        elif [[ $answer == [Nn] ]]; then
            read -p "Specify the aarch64-none-elf compiler path: " GCC_PATH
        fi
    done
fi

rm -rf "$RPI_PATH"/*
cp -rf scripts/config.txt "$RPI_PATH/config.txt"

mkdir "$RPI_PATH/overlays"
mkdir "$RPI_PATH/dtb"

wget https://github.com/timanu8/armstubsRPI4/raw/refs/heads/master/armstub8-gic.bin -P $RPI_PATH
wget https://github.com/raspberrypi/firmware/raw/refs/heads/master/boot/fixup4.dat -P $RPI_PATH
wget https://github.com/raspberrypi/firmware/raw/refs/heads/master/boot/bootcode.bin -P $RPI_PATH
wget https://github.com/raspberrypi/firmware/raw/refs/heads/master/boot/start4.elf -P $RPI_PATH
wget https://github.com/raspberrypi/firmware/raw/refs/heads/master/boot/bcm2711-rpi-4-b.dtb -P $RPI_PATH/dtb

wget https://github.com/raspberrypi/firmware/archive/refs/heads/master.zip
unzip -j master.zip "firmware-master/boot/overlays/*" -d $RPI_PATH/overlays
rm -rf master.zip