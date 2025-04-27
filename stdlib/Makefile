ARMGNU ?= aarch64-none-elf
FILES = file.cpp

all: build

build:
	aarch64-none-elf-g++ -ffreestanding -nostdlib -fPIC -shared -o libveil.so file.cpp