ARMGNU ?= aarch64-none-elf

COPS = -Wall -O0 -ffreestanding -nostdlib -nostartfiles -mstrict-align -Iinclude
CPPOPS = $(COPS) -std=c++20 -fno-exceptions -fno-rtti -Wno-write-strings
ASMOPS = -Iinclude

BUILD_DIR = build
SRC_DIR = src

ASM_DIR = $(SRC_DIR)/asm
DRIVERS_DIR = $(SRC_DIR)/drivers
LIB_DIR = $(SRC_DIR)/lib
FS_DIR = $(SRC_DIR)/fs
VFS_DIR = $(FS_DIR)/vfs

all : clean kernel8.img

dump:
	$(ARMGNU)-objdump -D build/kernel8.elf > dump

clean :
	rm -rf $(BUILD_DIR) *.img dump

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_cpp.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(ARMGNU)-g++ $(CPPOPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(ASM_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c $(DRIVERS_DIR)/*.c $(LIB_DIR)/*.c $(FS_DIR)/*.c $(VFS_DIR)/*.c)
CPP_FILES = $(wildcard $(SRC_DIR)/*.cpp $(DRIVERS_DIR)/*.cpp $(LIB_DIR)/*.cpp $(FS_DIR)/*.cpp $(VFS_DIR)/*.cpp)
ASM_FILES = $(wildcard $(ASM_DIR)/*.S)

OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(CPP_FILES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%_cpp.o)
OBJ_FILES += $(ASM_FILES:$(ASM_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img
	./go_rpi.sh
