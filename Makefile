.PHONY: clean

NETLIB  ?= net-nng.o

CFLAGS  := -Werror -g
LD      := gcc
LDLIBS  := ${LDLIBS} -libverbs -lpthread -lnng

APPS    := client server
COMMON  := config.o common.o $(NETLIB)

all: ${APPS}

client: client.c $(COMMON)
	${LD} -o $@ $^ ${LDLIBS}

server: server.c $(COMMON)
	${LD} -o $@ $^ ${LDLIBS}

clean:
	rm -f *.o ${APPS}

