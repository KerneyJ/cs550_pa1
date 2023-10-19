CC=gcc
CPP=g++

CPP_FLAGS=

BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src

PEER=peer_cli.cpp
C_PEER=c_peer
D_PEER=d_peer
IDXSVR=index_server.cpp
TEST=test.c
C_SRCS=comms.c
CPP_SRCS=server.cpp thread_pool.cpp file_index.cpp messages.cpp centralized_peer.cpp decentralized_peer.cpp benchmarks.cpp
C_OBJS=$(C_SRCS:.c=.o)
CFLAGS=
CPP_OBJS=$(CPP_SRCS:.cpp=.o)

ifeq ($(DEBUG),true)
    CFLAGS := -D DEBUG
    CPP_FLAGS := $(CPP_FLAGS) -D DEBUG
endif

all: $(C_PEER) $(D_PEER) $(IDXSVR)

$(TEST): $(C_OBJS)
	$(CC) $(SRC_DIR)/$(TEST) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) -o $(BIN_DIR)/test

$(IDXSVR): $(C_OBJS) $(CPP_OBJS) dir
	$(CPP) $(CPP_FLAGS) $(SRC_DIR)/$(IDXSVR) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) -o $(BIN_DIR)/index_server

$(C_PEER): $(C_OBJS) $(CPP_OBJS) dir
	$(CPP) -D CENTRALIZED_PEER $(CPP_FLAGS) $(SRC_DIR)/$(PEER) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) -o $(BIN_DIR)/$(C_PEER)

$(D_PEER): $(C_OBJS) $(CPP_OBJS) dir
	$(CPP) -D DECENTRALIZED_PEER $(CPP_FLAGS) $(SRC_DIR)/$(PEER) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) -o $(BIN_DIR)/$(D_PEER)

$(CPP_OBJS): %.o: $(SRC_DIR)/%.cpp dir
	$(CPP) $(CPP_FLAGS) -c $< -o $(OBJ_DIR)/$@

$(C_OBJS): %.o: $(SRC_DIR)/%.c dir
	$(CC) -c $< -o $(OBJ_DIR)/$@ $(CFLAGS)

dir:
	mkdir -p bin
	mkdir -p obj
	mkdir -p data
	mkdir -p config

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(OBJ_DIR)/*
