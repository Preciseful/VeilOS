ARMGNU ?= aarch64-none-elf

COPS = -Wall -O0 -ffreestanding -nostdlib -nostartfiles -mstrict-align -Iinclude
ASMOPS = -Iinclude 

BUILD_DIR = build
SRC_DIR = src

ASM_DIR = $(SRC_DIR)/asm
DRIVERS_DIR = $(SRC_DIR)/drivers
LIB_DIR = $(SRC_DIR)/lib

all : clean kernel8.img

clean :
	rm -rf $(BUILD_DIR) *.img 

qemu :
	sudo apt install python3
	wget https://download.qemu.org/qemu-9.2.0-rc0.tar.xz
	tar xvJf qemu-9.2.0-rc0.tar.xz
	cd qemu-9.2.0-rc0
	./configure
	make -j4

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(ASM_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c $(DRIVERS_DIR)/*.c $(LIB_DIR)/*.c)
ASM_FILES = $(wildcard $(ASM_DIR)/*.S)

OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(ASM_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img
	./go_rpi.sh