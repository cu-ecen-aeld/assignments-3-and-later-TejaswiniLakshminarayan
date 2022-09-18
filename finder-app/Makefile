CROSS_COMPILE = gcc
CFLAGS = -g -Wall
all: build

clean: 
	rm -f *.o writer *.elf *.map	
build:
	$(CROSS_COMPILE) $(CFLAGS)  -c -o writer.o writer.c
	$(CROSS_COMPILE) $(CFLAGS) -I/ writer.o -o writer

	
