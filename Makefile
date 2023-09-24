CC=gcc
CPP=g++

BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src

PEER=peer.cpp
IDXSVR=index_server.cpp
C_SRCS=comms.c 
CPP_SRCS=server.cpp thread_pool.cpp
C_OBJS=$(C_SRCS:.c=.o)
CPP_OBJS=$(CPP_SRCS:.cpp=.o)

all: $(PEER) $(IDXSVR)

$(IDXSVR): $(C_OBJS) $(CPP_OBJS) 
	$(CPP) $(SRC_DIR)/$(IDXSVR) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) -o $(BIN_DIR)/index_server

$(PEER): $(C_OBJS) $(CPP_OBJS)
	$(CPP) $(SRC_DIR)/$(PEER) $(addprefix $(OBJ_DIR)/,$(C_OBJS)) $(addprefix $(OBJ_DIR)/,$(CPP_OBJS)) -o $(BIN_DIR)/peer

$(CPP_OBJS): %.o: $(SRC_DIR)/%.cpp
	$(CPP) -c $< -o $(OBJ_DIR)/$@

$(C_OBJS): %.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $(OBJ_DIR)/$@

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(OBJ_DIR)/*
