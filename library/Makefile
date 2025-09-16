FILES=stdio.c stdlib.c svc.c stdfile.c portal.c

all: build

OBJS = $(addprefix bin/,$(FILES:.c=.o))

bin/%.o: %.c
	@mkdir -p bin
	$(ARMGNU)-gcc -c -ffreestanding -nostdlib -o $@ $<

build: $(OBJS)
	ar rcs libveil.a $(OBJS)