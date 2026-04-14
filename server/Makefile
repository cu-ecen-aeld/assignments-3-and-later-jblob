CROSS_COMPILE ?=

CC=$(CROSS_COMPILE)gcc -Wall -Werror

all: aesdsocket

aesdsocket: aesdsocket.o
	$(CC) -o aesdsocket aesdsocket.o

writer.o : aesdsocket.c
	$(CC) -c aesdsocket.c

clean :
	rm -f aesdsocket aesdsocket.o
