#!/bin/bash

VOIDELLE_PATH=""
VOIDELLE_ANSWER=""

if command -v voidelle > /dev/null; then
    while [[ $VOIDELLE_ANSWER != [YyNn] ]]; do
        read -p "Voidelle found. Use this as the second partition? (Y/n): " VOIDELLE_ANSWER
        VOIDELLE_ANSWER=${VOIDELLE_ANSWER:-"Y"}

        if [[ $VOIDELLE_ANSWER == [Yy] ]]; then
            VOIDELLE_PATH="voidelle"
        fi
    done
fi

if [[ $VOIDELLE_PATH == "" && $VOIDELLE_ANSWER != [Nn] ]]; then
    read -p "Voidelle not found. Installation is recommended, but the path can be left blank to use FAT32. Specify installation path: " VOIDELLE_PATH
    if [ -d "$VOIDELLE_PATH" ]; then
        VOIDELLE_PATH="$VOIDELLE_PATH/voidelle"
    fi
fi

dd if=/dev/zero of=/tmp/disk.img bs=1G count=5

(echo o; echo n; echo p; echo 1; echo 18432; echo +512M; echo n; echo p; echo 2; echo 1067008; echo 10485759; echo t; echo 1; echo 0c; echo w) | fdisk /tmp/disk.img

LOOP=$(sudo losetup --show -Pf /tmp/disk.img)
sudo chmod 666 "$LOOP"p1
sudo chmod 666 "$LOOP"p2

mkdosfs -F32 "$LOOP"p1 -n bootfs

if [[ -f "$VOIDELLE_PATH" || $VOIDELLE_PATH == "voidelle" ]]; then
    "$VOIDELLE_PATH" -s /tmp --init --disk="$LOOP"p2 --user=1
    echo "MAINFSFUNC=1" >> conf
else
    mkdosfs -F32 "$LOOP"p2 -n rootfs
    echo "MAINFSFUNC=2" >> conf
fi

sudo losetup -d "$LOOP"
sudo umount "$1"*

sudo dd if=/tmp/disk.img of=$1 bs=4M count=256 status=progress oflag=direct