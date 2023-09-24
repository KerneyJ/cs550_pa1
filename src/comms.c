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
	printf("[+]Server socket created successfully at %s:%d.\n", ip, port);

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
		error_string[255] = 0;
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

int createupdt_msg(msg_t* msg, char* update_message, int len){
	msg->buf = (char*)malloc(UPDATE_MSG_SIZE * sizeof(char));
	if(msg->buf == NULL){
		perror("[-]Error mallocing for update message buffer");
		return -1;
	}
	memcpy(msg->buf, update_message, len);
	msg->size = UPDATE_MSG_SIZE;
	msg->type = update;
	return 0;
}

int createfile_msg(msg_t* msg, char* path){
	int fd;
	struct stat s;

	fd = open(path, O_RDONLY);
	if(fd < 0){
		char error_string[256];
		sprintf(error_string, "[-]Error in createfile_msg open(%s)", path);
		error_string[255] = 0;
		perror(error_string);
	}
	fstat(fd, &s);
	msg->type = fd;
	msg->size = s.st_size;
	msg->buf = (char*)mmap(NULL, msg->size, PROT_READ, MAP_SHARED, fd, 0);
	if(msg->buf == MAP_FAILED){
		perror("[-]MMAP error while creating file message");
		return -1;
	}
	printf("[+]Successfully created a file message\n");
	return 0;
}

int send_msg(msg_t msg, conn_t conn){ // TODO might need to use protobuff for message layout
	printf("[*]Attempting to send %s\n", msg.buf);
	size_t bytestosend = msg.size, bufferroom = SENDSIZE;
	char sendbuf[SENDSIZE] = {0}, *ptr;
	ptr = sendbuf;
	memcpy(ptr, (char*)(&msg.type), sizeof(int));
	ptr += sizeof(int);
	memcpy(ptr, (char*)(&msg.size), sizeof(size_t));
	bufferroom -= sizeof(int) + sizeof(size_t);
	ptr += sizeof(size_t);
	while(bytestosend){
		memcpy(ptr, msg.buf, bytestosend - bufferroom);
		if(send(conn.sock, sendbuf, SENDSIZE, 0) < 0){
			perror("[-]Error in sending message");
			return -1;
		}
		printf("[+]Sent message chunk %s\n");
		bzero(sendbuf, SENDSIZE);
		ptr = sendbuf;
		bytestosend -= bufferroom;
	}
	printf("[+]Successfully sent entire message\n");
	return 0;
}


// TODO make return msg_t
int recv_msg(conn_t conn, char* buf){
	int bytesread = recv(conn.sock, buf, SENDSIZE, 0);
	if(bytesread < 0)
		perror("[-]Error in receiving message");
	return bytesread;
}
