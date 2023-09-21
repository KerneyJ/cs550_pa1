#include <stdio.h>
#include "comms.h"

int servinit_conn(conn_t* conn, char* ip, int port){
	struct sockaddr_in serv_addr;
	socklen_t addr_size;
	conn->sock = socket(AF_INET, SOCK_STREAM, 0);
	conn->port = port;
	conn->addr = inet_addr(ip);
	if(conn->sock < 0){
		perror("[-] Error on socket creation");
		return -1;
	}
	printf("[+]Server socket created successfully.\n");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = conn->port;
	serv_addr.sin_addr.s_addr = conn->addr;

	if(bind(conn->sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		perror("[-]Error in bind");
		return -1;
	}
	printf("[+]Binding successfull.\n");
	return 0;
}

int servlstn_conn(conn_t* conn, int backlog){
	if(listen(conn->sock, backlog) == 0)
		printf("[+]Listening...\n");
	else{
		perror("[-]Error in listening");
		return -1;
	}
	return 0;
}

conn_t servacpt_conn(conn_t* conn){
	conn_t client_conn = {0, 0, 0};
	struct sockaddr_in client_addr;
	socklen_t client_size = sizeof(client_addr);
	client_conn.sock = accept(conn->sock, (struct sockaddr*)&client_addr, &client_size);
	if(client_conn.sock < 0){
		perror("[-]Error on accept");
		client_conn.addr = -1;
		client_conn.port = -1;
		client_conn.sock = -1;
		return client_conn;
	}
	client_conn.addr = client_addr.sin_addr.s_addr;
	client_conn.port = client_addr.sin_port;

	unsigned char* ip = (unsigned char*)&client_conn.addr;
	printf("[+]Accepted connection at %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], client_conn.port);

	return client_conn;
}

int clntinit_conn(conn_t* conn, char* ip, int port){
	struct sockaddr_in serv_addr;
	conn->sock = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = port;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	if(connect(conn->sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		char error_string[256];
		sprintf(error_string,"[-]Error connecting to %s:%d", ip, port);
		perror(error_string);
		return -1;
	}
	printf("[+]Connected to Server.\n");
	return 0;
}

int close_conn(conn_t *conn){
	close(conn->sock);
	conn->sock = -1;
	conn->port = -1;
	conn->addr = -1;
	return 0;
}
