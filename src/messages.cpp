#include "messages.hpp"
#include <cstring>

#define IP_INFO_SIZE sizeof(int) * 2

int send_once(conn_t dest, msg_t message) {
	conn_t client;

	clntinitco_conn(&client, &dest);

	if (send_msg(message, client) < 0) {
		close_conn(&client);
		return -1;
	}

	close_conn(&client);
	return 0;
}

msg_t send_and_recv(conn_t dest, msg_t request) {
    conn_t client;
    msg_t response;
    
	clntinitco_conn(&client, &dest);
    
	if (send_msg(request, client) < 0) {
		close_conn(&client);
		return { nullptr, 0, NULL_MSG };
	}

	response = recv_msg(client);
	close_conn(&client);

    return response;
}

void create_message(msg_t* msg, msg_type type) {
	createupdt_msg(msg, NULL, 0, type);
}

void create_message(msg_t* msg, std::string str, conn_t conn, msg_type type) {
	int *ibuffer;

	msg->buf = (char*) malloc(IP_INFO_SIZE + str.length());
	ibuffer = (int*) msg->buf;  
	ibuffer[0] = conn.addr;
	ibuffer[1] = conn.port;
	memcpy(msg->buf + IP_INFO_SIZE, str.data(), str.length());

	msg->size = IP_INFO_SIZE + str.length();
	msg->type = type;
}

void create_message(msg_t* msg, conn_t conn, msg_type type) {
	int buffer[2] = {conn.addr, conn.port};
	createupdt_msg(msg, (char*) buffer, IP_INFO_SIZE, type);
}

void create_message(msg_t* msg, std::string str, msg_type type) {
	msg->buf = (char*) malloc(str.length());
	memcpy(msg->buf, str.c_str(), str.length());

	msg->size = str.length();
	msg->type = type;
}

void create_message(msg_t* msg, int id, conn_t conn, msg_type type) {
 	int buffer[3] = {id, conn.addr, conn.port};
	createupdt_msg(msg, (char*) buffer, IP_INFO_SIZE + sizeof(int), type);
}

void create_message(msg_t* msg, int id, std::string str, msg_type type) {
	int *ibuffer;

	msg->buf = (char*) malloc(sizeof(int) + str.length());
	ibuffer = (int*) msg->buf;  
	ibuffer[0] = id;
	memcpy(msg->buf + sizeof(int), str.data(), str.length());

	msg->size = sizeof(int) + str.length();
	msg->type = type;
}



void parse_message(msg_t* msg, conn_t* conn) {
	int *ibuf = (int*) msg->buf;
	conn->addr = ibuf[0];
	conn->port = ibuf[1];
}

void parse_message(msg_t* msg, std::string* str) {
	str->assign(msg->buf, msg->buf + msg->size);
}

void parse_message(msg_t* msg, std::string* str, conn_t* conn) {
	str->assign(msg->buf + IP_INFO_SIZE, msg->buf + msg->size);

	int* ibuffer = (int*) msg->buf;
	conn->addr = ibuffer[0];
	conn->port = ibuffer[1];
}

void parse_message(msg_t* msg, int* id, conn_t* conn) {
	int *ibuf = (int*) msg->buf;
	*id = ibuf[0];
	conn->addr = ibuf[1];
	conn->port = ibuf[2];
}

void parse_message(msg_t* msg, int* id, std::string* str) {
	int *ibuf = (int*) msg->buf;
	*id = ibuf[0];
	str->assign(msg->buf + sizeof(int), msg->buf + msg->size);
}

void parse_message(msg_t* msg, int* id) {
	int *ibuf = (int*) msg->buf;
	*id = ibuf[0];
}