#include "messages.hpp"
#include "comms.h"

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

msg_t str_and_conn_to_msg(std::string str, conn_t conn, msg_type type) {
	msg_t msg;
	int *ibuffer;

	msg.buf = (char*) malloc(IP_INFO_SIZE + str.length());
	ibuffer = (int*) msg.buf;  
	ibuffer[0] = conn.addr;
	ibuffer[1] = conn.port;
	memcpy(msg.buf + IP_INFO_SIZE, str.data(), str.length());

	msg.size = IP_INFO_SIZE + str.length();
	msg.type = type;

    return msg;
}

msg_t conn_to_msg(conn_t conn, msg_type type) {
	msg_t msg;

	int buffer[2] = {conn.addr, conn.port};
	createupdt_msg(&msg, (char*) buffer, IP_INFO_SIZE, type);

    return msg;
}

msg_t str_to_msg(std::string str, conn_t conn, msg_type type) {
    msg_t msg;

	msg.buf = (char*) malloc(str.length());
	memcpy(msg.buf, str.data(), str.length());

	msg.size = str.length();
	msg.type = type;

    return msg;
}

conn_t msg_to_conn(msg_t msg) {
	int *ibuf = (int*) msg.buf;
	int host_ip = ibuf[0];
	int host_port = ibuf[1];

    return { host_ip, host_port, 0 };
}

std::string msg_to_str(msg_t msg) {
	std::string str(&msg.buf, &msg.buf + msg.size);
	return str;
}

std::pair<std::string, conn_t> msg_to_str_and_conn(msg_t msg) {
	conn_t conn;

	std::string str(&msg.buf + IP_INFO_SIZE, &msg.buf + msg.size - IP_INFO_SIZE);

	int* ibuffer = (int*) msg.buf;
	conn.addr = ibuffer[0];
	conn.port = ibuffer[1];

	return { str, conn };
}