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

int createupdt_msg(msg_t* msg, char* update_message, int len, int type){
	msg->buf = (char*)malloc(UPDATE_MSG_SIZE * sizeof(char));
	if(msg->buf == NULL){
		perror("[-]Error mallocing for update message buffer");
		return -1;
	}
	memcpy(msg->buf, update_message, len);
	msg->size = UPDATE_MSG_SIZE;
	msg->type = type;
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

int delete_msg(msg_t* msg){
	if(IS_FILE_MSG(msg->type)){
		int fd, munmap_ret;
		fd = msg->type;
		munmap_ret = munmap(msg->buf, msg->size);
		if(munmap_ret < 0){
			perror("[-]Error munmap in delete msg");
			return -1;
		}
		if(close(fd) < 0){
			perror("[-]Error on closing msg file descriptor");
			return -1;
		}
		msg->buf = NULL;
		msg->size = -1;
		msg->type = NULL_MSG;
	}
	else{
		msg->size = -1;
		msg->type = NULL_MSG;
		if(msg->buf !=NULL){
			free(msg->buf);
			msg->buf = NULL;
		}
	}
	return 0;
}

int send_msg(msg_t msg, conn_t conn){ // TODO might need to use protobuff for message layout
	printf("[*]Attempting to send message\n");
	size_t bytestosend = msg.size, bufferroom = SENDSIZE, sent, sending;
	char sendbuf[SENDSIZE] = {0}, *bufpos;
	bufpos = sendbuf;
	memcpy(bufpos, (char*)(&msg.type), sizeof(int));
	bufpos += sizeof(int);
	memcpy(bufpos, (char*)(&msg.size), sizeof(size_t));
	bufpos += sizeof(size_t);
	bufferroom -= (sizeof(int) + sizeof(size_t));
	memcpy(bufpos, msg.buf, bufferroom);
	
	// initial send
	sent = send(conn.sock, sendbuf, SENDSIZE, 0);
	if(sent < 0){
		perror("[-]Error on first packet of send message");
		return -1;
	}
	bzero(sendbuf, SENDSIZE);
	bytestosend -= bufferroom;
	bufpos = msg.buf + bufferroom;
	printf("[+]Successfully sent first packet");

	while(bytestosend > 0){
		if(SENDSIZE < bytestosend)
			sending = SENDSIZE;
		else
			sending = bytestosend;

		memcpy(sendbuf, bufpos, sending);
		sent = send(conn.sock, sendbuf, sending, 0);
		if(sent < 0){
			perror("[-]Error in sending message");
			return -1;
		}
		bzero(sendbuf, SENDSIZE);
		bufpos += sent;
		bytestosend -= sent;
		printf("[+]Sent %i bytes of %i\n", sent, bytestosend);
	}
	printf("[+]Successfully sent entire message\n");
	return 0;
}

msg_t recv_msg(conn_t conn){
	int type, bytesread, bytesleft, fd;
	size_t size = 0;
	msg_t ret;
	char recvbuf[SENDSIZE] = {0}, *bufpos;

	// read initial message
	bytesread = recv(conn.sock, recvbuf, SENDSIZE, 0);
	if(bytesread < 0){
		perror("[-]Error in receiving initial part of message");
		ret.buf = NULL; ret.size = 0; ret.type = NULL_MSG;
		return ret;
	}

	// parse type and size of msg_t from the first message
	type = ((int*)recvbuf)[0];
	size = ((size_t*)(recvbuf+sizeof(int)))[0];
	ret.type = type;
	ret.size = size;

	// allocate memory for message depending on type of message
	if(IS_FILE_MSG(ret.type)){
		// TODO create a loop that checks for names for the tmp file
		// this sounds expensive, maybe specify a file name in the message
		fd = open("./tmp", O_RDWR | O_CREAT, 0644);
		if(fd < 0){
			perror("[-]Error creating temp file for file msg");
			ret.type = NULL_MSG; ret.buf = NULL; ret.size = 0;
			return ret;
		}
		if(ftruncate(fd, ret.size) < 0){
			perror("[-]Error increasing size of tmp in recv_msg");
			ret.type = NULL_MSG; ret.buf = NULL; ret.size = 0;
			close(fd);
			return ret;
		}
		ret.buf = mmap(NULL, ret.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if(ret.buf == MAP_FAILED){
			perror("[-]Error on mmap in creating file for recv");
			close(fd);
			ret.type = NULL_MSG; ret.buf = NULL; ret.size = 0;
			return ret;
		}
		ret.type = fd;
	}
	else{
		if(ret.size == 0)
			ret.buf = NULL;
		else
			ret.buf = (char*)malloc(ret.size);
	}

	// calculate the number of bytes left to read
	// sizeof(int) => msg_t.type
	// sizeof(size_t) => msg_t.size
	// size => number of bytes contained in the message on the other side
	bytesleft = (size + sizeof(int) + sizeof(size_t)) - bytesread;
	printf("[+]Received message of type %d and size %lu\n", type, size);
	
	// copy bytes read after the header into ret buffer
	memcpy(ret.buf, recvbuf+(sizeof(int) + sizeof(size_t)), bytesread-(sizeof(int) + sizeof(size_t)));
	bufpos = ret.buf + (bytesread - (sizeof(int)+sizeof(size_t)));
	bzero(recvbuf, SENDSIZE);
	while(bytesleft > 0){
		bytesread = recv(conn.sock, recvbuf, SENDSIZE, 0);

		// error handling if recv fails
		if(bytesread < 0){
			perror("[-] Error in receiving part of message");
			if(IS_FILE_MSG(ret.type)){
				if(munmap(ret.buf, ret.size)){
					perror("[-]Error munmap in Error of recv in loop");
					ret.buf = NULL; ret.size = 0; ret.type = NULL_MSG;
					return ret;
				}
				close(ret.type);
			}
			else{
				if(ret.buf != NULL)
					free(ret.buf);
			}
			ret.buf = NULL; ret.size = 0; ret.type = NULL_MSG;
			return ret;
		}

		// copy bytes read into ret buf
		memcpy(bufpos, recvbuf, bytesread);
		bzero(recvbuf, SENDSIZE);
		bufpos += bytesread;
		bytesleft -= bytesread;
		printf("[+]Successfully read and copied %i bytes, %i bytes left\n", bytesread, bytesleft);
		if(bytesread == 0)
			break;
	}
	printf("[+]Successfully read entire message\n");
	return ret;
}
