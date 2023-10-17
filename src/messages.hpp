#include <cstdint>
#include <string>

extern "C" {
    #include "comms.h"
}

int send_once(conn_t conn, msg_t message);
msg_t send_and_recv(conn_t conn, msg_t message);

void create_message(msg_t* msg, msg_type);
void create_message(msg_t* msg, conn_t conn, msg_type);
void create_message(msg_t* msg, std::string str, msg_type);
void create_message(msg_t* msg, std::string str, conn_t conn, msg_type);
void create_message(msg_t* msg, int id, conn_t conn, msg_type);
void create_message(msg_t* msg, int id, std::string str, msg_type);

void parse_message(msg_t* msg, conn_t* conn);
void parse_message(msg_t* msg, std::string* str);
void parse_message(msg_t* msg, std::string* str, conn_t* conn);
void parse_message(msg_t* msg, int* id, conn_t* conn);
void parse_message(msg_t* msg, int* id, std::string* str);
void parse_message(msg_t* msg, int* id);