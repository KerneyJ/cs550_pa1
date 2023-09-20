CC=gcc
CPP=g++

PEER=peer.bin
SRCS=comms.c
OBJS=$(SRCS:.c=.o)

all: $(PEER)

$(PEER): $(OBJS)
	$(CC) peer.c $(OBJS) -o $@

$(OBJS): $(SRCS)
	$(CC) -c $< -o $@

clean:
	rm *.o
