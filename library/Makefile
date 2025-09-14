FILES=stdio.c stdlib.c svc.c stdfile.c portal.c

all: build

OBJS = $(FILES:.c=.o)

%.o: %.c
	$(ARMGNU)-gcc -c -ffreestanding -nostdlib -o $@ $<

build: $(OBJS)
	ar rcs libveil.a $(OBJS)