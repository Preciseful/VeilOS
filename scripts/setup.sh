RPI_PATH=$1

rm -rf "$RPI_PATH"/*
cp -rf scripts/config.txt "$RPI_PATH/config.txt"

mkdir -p "$RPI_PATH/kernel/modules"
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