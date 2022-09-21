CC = $(CROSS_COMPILE)gcc
CFLAGS = -g -Wall
all: build

clean: 
	rm -f *.o writer *.elf *.map	
build:
	$(CC) $(CFLAGS)  -c -o writer.o writer.c
	$(CC) $(CFLAGS) -I/ writer.o -o writer

	
