FILES = $(shell find . -type f -name "*.c" -print | cut -d'/' -f2-)

all: build

OBJS = $(addprefix bin/,$(FILES:.c=.o))

bin/%.o: %.c
	@mkdir -p $(dir $@)
	$(ARMGNU)-gcc -I . -c -ffreestanding -nostdlib -o $@ $<

build: $(OBJS)
	ar rcs libveil.a $(OBJS)