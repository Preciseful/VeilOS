ARMGNU ?= aarch64-none-elf
FILES = filesystem.cpp

all: build

build:
	aarch64-none-elf-g++ -Wno-return-type -ffreestanding -nostdlib -fPIC -shared -o libveil.so $(FILES)