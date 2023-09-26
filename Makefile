CC=gcc
CPP=g++

BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src

PEER_CLI=peer_cli.cpp
PEER_SERVER=peer_server.cpp
IDXSVR=index_server.cpp
TEST=test.c
C_SRCS=comms.c
CPP_SRCS=server.cpp thread_pool.cpp file_index.cpp peer.cpp
C_OBJS=$(C_SRCS:.c=.o)
CPP_OBJS=$(CPP_SRCS:.cpp=.o)

all: $(PEER) $(PEER_SERVER) $(IDXSVR)

$(TEST): $(C_OBJS)
	$(CC) $(SRC_DIR)/$(TEST) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) -o $(BIN_DIR)/test

$(IDXSVR): $(C_OBJS) $(CPP_OBJS)
	$(CPP) $(SRC_DIR)/$(IDXSVR) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) -o $(BIN_DIR)/index_server

$(PEER): $(C_OBJS) $(CPP_OBJS)
	$(CPP) $(SRC_DIR)/$(PEER) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) -o $(BIN_DIR)/peer_cli

$(PEER_SERVER): $(C_OBJS) $(CPP_OBJS)
	$(CPP) $(SRC_DIR)/$(PEER_SERVER) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) -o $(BIN_DIR)/peer_server

$(CPP_OBJS): %.o: $(SRC_DIR)/%.cpp
	$(CPP) -c $< -o $(OBJ_DIR)/$@

$(C_OBJS): %.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $(OBJ_DIR)/$@

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(OBJ_DIR)/*
