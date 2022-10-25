#CC=$(CROSS_COMPILE)gcc
#CFLAGS = -g -Wall -Werror -pthread

ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($(CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif
ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lrt
endif
all:aesdsocket

clean:
	rm -f *.o aesdsocket *.elf *.map

aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LDFLAGS)
