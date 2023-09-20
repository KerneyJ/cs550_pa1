CC=gcc
CPP=g++

SRCS=comms.c
OBJS=$(SRCS:.c=.o)

$(OBJS): $(SRCS)
	$(CC) -c $< -o $@

