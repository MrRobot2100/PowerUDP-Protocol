CC      = gcc
CFLAGS  = -Wall -Wextra -g
LDFLAGS = -lpthread

all: servidor cliente

servidor: servidor.c powerudp.c powerudp.h
	$(CC) $(CFLAGS) -o servidor servidor.c powerudp.c $(LDFLAGS)

cliente: cliente.c powerudp.c powerudp.h
	$(CC) $(CFLAGS) -o cliente cliente.c powerudp.c $(LDFLAGS)

clean:
	rm -f servidor cliente *.o

.PHONY: all clean
