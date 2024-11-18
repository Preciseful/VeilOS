ARMGNU ?= aarch64-none-elf

COPS = -Wall -O0 -ffreestanding -nostdlib -nostartfiles -mstrict-align -Iinclude
ASMOPS = -Iinclude 

BUILD_DIR = build
SRC_DIR = src

ASM_DIR = $(SRC_DIR)/asm
DRIVERS_DIR = $(SRC_DIR)/drivers
LIB_DIR = $(SRC_DIR)/lib

ifdef ALT_DIR
ALT_ASM_DIR = $(ALT_DIR)/asm
ALT_DRIVERS_DIR = $(ALT_DIR)/drivers
ALT_LIB_DIR = $(ALT_DIR)/lib
endif

all : clean kernel8.img

clean :
	rm -rf $(BUILD_DIR) *.img 

ifdef ALT_DIR
$(BUILD_DIR)/%_c.o: $(ALT_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(ALT_ASM_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@
endif

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(ASM_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c $(DRIVERS_DIR)/*.c $(LIB_DIR)/*.c)
ASM_FILES = $(wildcard $(ASM_DIR)/*.S)

ifdef ALT_DIR
ALT_SRC_C_FILES = $(wildcard $(ALT_DIR)/*.c)
ALT_DRIVERS_C_FILES = $(wildcard $(ALT_DRIVERS_DIR)/*.c)
ALT_LIB_C_FILES = $(wildcard $(ALT_LIB_DIR)/*.c)

ALT_ASM_FILES = $(wildcard $(ALT_ASM_DIR)/*.S)

SRC_OVERLAP_FILES = $(notdir $(ALT_SRC_C_FILES))
DRIVERS_OVERLAP_FILES = $(notdir $(ALT_DRIVERS_C_FILES))
LIB_OVERLAP_FILES = $(notdir $(ALT_LIB_C_FILES))

C_FILES := $(filter-out $(addprefix $(SRC_DIR)/, $(SRC_OVERLAP_FILES)), $(C_FILES))
C_FILES := $(filter-out $(addprefix $(DRIVERS_DIR)/, $(DRIVERS_OVERLAP_FILES)), $(C_FILES))
C_FILES := $(filter-out $(addprefix $(LIB_DIR)/, $(LIB_OVERLAP_FILES)), $(C_FILES))

ASM_FILES := $(filter-out $(addprefix $(ASM_DIR)/, $(notdir $(ALT_ASM_FILES))), $(ASM_FILES))
endif

OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(ASM_DIR)/%.S=$(BUILD_DIR)/%_s.o)

ifdef ALT_DIR
OBJ_FILES += $(ALT_ASM_FILES:$(ALT_ASM_DIR)/%.S=$(BUILD_DIR)/%_s.o)
OBJ_FILES += $(ALT_SRC_C_FILES:$(ALT_DIR)/%.c=$(BUILD_DIR)/%_c.o)
endif

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img
	./go_rpi.sh
