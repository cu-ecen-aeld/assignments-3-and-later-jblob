CROSS_COMPILE ?=
 
CC ?= $(CROSS_COMPILE)gcc 
CFLAGS ?= -g -Wall -Werror
TARGET ?= aesdsocket
LDFLAGS ?= -lpthread -lrt

all: $(TARGET)

aesdsocket: aesdsocket.o
	$(CC) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LDFLAGS)

clean :
	rm -f aesdsocket aesdsocket.o
