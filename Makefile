RPI_PATH=/media/$(USER)/bootfs

ARMGNU ?= aarch64-none-elf

COPS = -fPIE -Wall -O0 -ffreestanding -nostdlib -nostartfiles -mstrict-align -Ikernel -mgeneral-regs-only
CPPOPS = $(COPS) -std=c++20 -fno-exceptions -fno-rtti -Wno-write-strings
ASMOPS = -Ikernel -fPIE

BUILD_DIR = build/bin
SRC_DIR = kernel

all : clean kernel8.img

lib:
	$(MAKE) -C library ARMGNU=$(ARMGNU)

setup:
	scripts/setup.sh $(RPI_PATH)

dump:
	$(ARMGNU)-objdump -D $(BUILD_DIR)/kernel8.elf > dump

clean :
	@rm -rf $(BUILD_DIR) *.img dump

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_cpp.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	@$(ARMGNU)-g++ $(CPPOPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	@mkdir -p $(@D)
	@$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@


C_FILES = $(shell find . \( -path ./library \) -prune -o -type f -name "*.c" -print | cut -d'/' -f2-)
CPP_FILES = $(shell find . \( -path ./library \) -prune -o -type f -name "*.cpp" -print | cut -d'/' -f2-)
ASM_FILES = $(shell find . \( -path ./library \) -prune -o -type f -name "*.S" -print | cut -d'/' -f2-)

OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(CPP_FILES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%_cpp.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: linker.ld $(OBJ_FILES)
	@$(ARMGNU)-ld -pie -T linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES)
	@$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img
	scripts/go_rpi.sh $(RPI_PATH)