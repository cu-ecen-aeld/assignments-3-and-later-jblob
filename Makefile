# CROSS_COMPILE ?= aarch64-none-linux-gnu-
CROSS_COMPILE ?= 

CC = $(CROSS_COMPILE)gcc

writer : finder-app/writer.o
	$(CC) -o finder-app/writer finder-app/writer.o

writer.o : finder-app/writer.c
	$(CC) -c finder-app/writer.c

clean :
	rm finder-app/writer finder-app/writer.o
