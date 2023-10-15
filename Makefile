CC=gcc
CPP=g++

CPP_FLAGS=

BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src

PEER=peer_cli.cpp
IDXSVR=index_server.cpp
TEST=test.c
C_SRCS=comms.c
CPP_SRCS=server.cpp thread_pool.cpp file_index.cpp messages.cpp centralized_peer.cpp
C_OBJS=$(C_SRCS:.c=.o)
CFLAGS=
CPP_OBJS=$(CPP_SRCS:.cpp=.o)

ifeq ($(DEBUG),true)
    CFLAGS := -D DEBUG
    CPP_FLAGS := $(CPP_FLAGS) -D DEBUG
endif

all: $(PEER) $(IDXSVR)

$(TEST): $(C_OBJS)
	$(CC) $(SRC_DIR)/$(TEST) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) -o $(BIN_DIR)/test

$(IDXSVR): $(C_OBJS) $(CPP_OBJS)
	$(CPP) $(CPP_FLAGS) $(SRC_DIR)/$(IDXSVR) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) -o $(BIN_DIR)/index_server

$(PEER): $(C_OBJS) $(CPP_OBJS)
	$(CPP) $(CPP_FLAGS) $(SRC_DIR)/$(PEER) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) -o $(BIN_DIR)/peer

$(CPP_OBJS): %.o: $(SRC_DIR)/%.cpp
	$(CPP) $(CPP_FLAGS) -c $< -o $(OBJ_DIR)/$@

$(C_OBJS): %.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $(OBJ_DIR)/$@ $(CFLAGS)

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(OBJ_DIR)/*
