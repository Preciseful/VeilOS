C_FILES = $(shell find . -type f -name "*.c" -print | cut -d'/' -f2-)
CPP_FILES += $(shell find . -type f -name "*.cpp" -print | cut -d'/' -f2-)


BUILD_DIR = bin

all: build

OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%_c.o)
OBJS += $(CPP_FILES:%.cpp=$(BUILD_DIR)/%_cpp.o)

$(BUILD_DIR)/%_c.o: %.c
	@mkdir -p $(dir $@)
	$(ARMGNU)-gcc -I . -c -ffreestanding -nostdlib -o $@ $<

$(BUILD_DIR)/%_cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(ARMGNU)-g++ -I . -c -ffreestanding -nostdlib -o $@ $<

build: $(OBJS)
	ar rcs libveil.a $(OBJS)