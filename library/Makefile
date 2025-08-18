FILES=stdio.c stdlib.c svc.c

all: build

OBJS = $(FILES:.c=.o)

%.o: %.c
	$(ARMGNU)-gcc -c -ffreestanding -nostdlib -fno-exceptions -fno-unwind-tables -fno-rtti -o $@ $<

build: $(OBJS)
	ar rcs libveil.a $(OBJS)