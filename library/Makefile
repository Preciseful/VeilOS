FILES=stdio.c stdlib.c svc.c

all: build

OBJS = $(FILES:.c=.o)

%.o: %.c
	$(ARMGNU)-gcc -c -ffreestanding -o $@ $<

build: $(OBJS)
	ar rcs library $(OBJS)
