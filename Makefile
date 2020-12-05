.PHONY: clean

NETLIB  ?= net.o
RMEMIMPL ?= rmem-rdma.c

CFLAGS  := -Werror -g
LD      := gcc
LDLIBS  := ${LDLIBS} -libverbs -lpthread -lnanomsg -ljpeg

APPS    := arrayclient arrayserver
COMMON  := config.o common.o rarray.o $(NETLIB) $(RMEMIMPL)

all: ${APPS}

arrayclient: arrayclient.c $(COMMON)
	${LD} -o $@ $^ ${LDLIBS}

arrayserver: arrayserver.c $(COMMON)
	${LD} -o $@ $^ ${LDLIBS}

clean:
	rm -f *.o ${APPS}

