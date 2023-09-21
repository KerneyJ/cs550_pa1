CC=gcc
CPP=g++

PEER=peer.bin
IDXSVR=index_server.bin
SRCS=comms.c
OBJS=$(SRCS:.c=.o)

all: $(PEER) $(IDXSVR)

$(IDXSVR): $(OBJS)
	$(CPP) index_server.cpp $(OBJS) -o $@

$(PEER): $(OBJS)
	$(CPP) peer.cpp $(OBJS) -o $@

$(OBJS): $(SRCS)
	$(CC) -c $< -o $@

clean:
	rm *.o
	rm *.bin
