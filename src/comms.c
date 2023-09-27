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
#ifdef DEBUG
	printf("[+]Binding successfull.\n");
#endif
	return 0;
}

int servlstn_conn(conn_t* conn, int backlog){
	if(listen(conn->sock, backlog) == 0)
#ifdef DEBUG
		printf("[+]Listening...\n");
#else
		;
#endif
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

int clntinitco_conn(conn_t* conn, conn_t* srv){
	struct sockaddr_in serv_addr;
	conn->sock = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = srv->port;
	serv_addr.sin_addr.s_addr = srv->addr;
	if(connect(conn->sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		char error_string[256];
		sprintf(error_string,"[-]Error connecting to %s:%d", srv->addr, srv->port);
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

inline int create_message(msg_t* msg, char* string, int type){
	if(type > 0)
		return createfile_msg(msg, string);
	else
		return createupdt_msg(msg, string, strlen(string), type);
}

int createupdt_msg(msg_t* msg, char* update_message, int len, int type){
	if(len){
		msg->buf = (char*)malloc(len * sizeof(char));
		if(msg->buf == NULL){
			perror("[-]Error mallocing for update message buffer");
			return -1;
		}
	}
	else{
		msg->buf = NULL;
	}
	memcpy(msg->buf, update_message, len);
	msg->size = len;
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
		return -1;
	}
	fstat(fd, &s);
	msg->type = fd;
	msg->size = s.st_size;
	msg->buf = (char*)mmap(NULL, msg->size, PROT_READ, MAP_SHARED, fd, 0);
	printf("🎂🎂🎂🎂\n");
	if(msg->buf == MAP_FAILED){
		perror("[-]MMAP error while creating file message\n");
		return -1;
	}
#ifdef DEBUG
	printf("[+]Successfully created a file message\n");
#endif
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

static void getname(char* absolute_path, int len){
	// parse the fd_path for the last slash
	int lastslash = 0, zeroout;
	for(int i = 0; i < len; i++){
		if(absolute_path[i] == '/')
			lastslash = i;
	}
	zeroout = len-lastslash;
	for(int i = 0; i < zeroout-1; i++){
		absolute_path[i] = absolute_path[i+lastslash+1];
	}
	memset(absolute_path+zeroout, 0, lastslash);
}

static int sendfile_msg(msg_t msg, conn_t conn){
	printf("[*]Attempting to send file message of size %li + 256(header)\n", msg.size, msg.type);
	long int bytestosend = msg.size;
	size_t bufferroom = SENDSIZE, sent, sending, headersize = 256;
	char sendbuf[SENDSIZE] = {0}, *bufpos, fd_path[256] = {0};
	char filepath[256-sizeof(int)-sizeof(size_t)] = {0};

	bufpos = sendbuf;
	memcpy(bufpos, (char*)(&msg.type), sizeof(int));
	bufpos += sizeof(int);

	memcpy(bufpos, (char*)(&msg.size), sizeof(size_t));
	bufpos += sizeof(size_t);

	// Get filename
	sprintf(fd_path, "/proc/self/fd/%d", msg.type);
	if(readlink(fd_path, filepath, sizeof(filepath)) < 0){
		perror("[-]Error on readlink on file msg fd in send_msg");
		return -1;
	}
	getname(filepath, sizeof(filepath));
	memcpy(bufpos, filepath, sizeof(filepath));
	bufpos = sendbuf + headersize;
	bufferroom = SENDSIZE - headersize;

	memcpy(bufpos, msg.buf, bufferroom);
	sent = send(conn.sock, sendbuf, sizeof(sendbuf), 0);
	if(sent < 0){
		perror("[-]Error on first packet of send message");
		return -1;
	}
	bzero(sendbuf, sizeof(sendbuf));
	bytestosend -= (sizeof(sendbuf) - headersize); // number of bytes left in msg.buf to send
	bufpos = msg.buf + (sizeof(sendbuf) - headersize); // bufpos is now a position of msg.buf instead of indexing sendbuf
#ifdef DEBUG
	printf("[+]Successfully sent first packet of size %d. bytes left: %d\n", sent, bytestosend);
#endif
	while(bytestosend > 0){
		if(SENDSIZE < bytestosend)
			sending = SENDSIZE;
		else
			sending = bytestosend;

		memcpy(sendbuf, bufpos, bufferroom);
		sent = send(conn.sock, sendbuf, sending, 0);
		if(sent < 0){
			perror("[-]Error in sending message");
			return -1;
		}
#ifdef DEBUG
		printf("[+]Sent %i bytes of %i\n", sent, bytestosend);
#endif
		bzero(sendbuf, SENDSIZE);
		bufpos += sent;
		bytestosend -= sent;
		bufferroom = SENDSIZE;
	}
	printf("[+]Successfully sent entire message\n");
	return 0;
}

static int sendupdt_msg(msg_t msg, conn_t conn){
	printf("[*]Attempting to send update message of size %li + sizeof(int) + sizeof(size_t)(header)\n", msg.size, msg.type);
	long int bytestosend = msg.size;
	size_t bufferroom = SENDSIZE, sent, sending, headersize = sizeof(int) + sizeof(size_t);
	char sendbuf[SENDSIZE] = {0}, *bufpos;

	bufpos = sendbuf;
	memcpy(bufpos, (char*)(&msg.type), sizeof(int));
	bufpos += sizeof(int);

	memcpy(bufpos, (char*)(&msg.size), sizeof(size_t));
	bufpos += sizeof(size_t);

	bufferroom -= headersize;
	if(msg.size < bufferroom)
		memcpy(bufpos, msg.buf, msg.size);
	else
		memcpy(bufpos, msg.buf, bufferroom);
	sent = send(conn.sock, sendbuf, sizeof(sendbuf), 0);
	if(sent < 0){
		perror("[-]Error on first packet of send message");
		return -1;
	}
	bzero(sendbuf, sizeof(sendbuf));
	bytestosend -= (sizeof(sendbuf) - headersize); // number of bytes left in msg.buf to send
	bufpos = msg.buf + (sizeof(sendbuf) - headersize); // bufpos is now a position of msg.buf instead of indexing sendbuf
#ifdef DEBUG
	printf("[+]Successfully sent first packet. bytes left: %d\n", bytestosend);
#endif
	while(bytestosend > 0){
		if(SENDSIZE < bytestosend)
			sending = SENDSIZE;
		else
			sending = bytestosend;

		memcpy(sendbuf, bufpos, bufferroom);
		sent = send(conn.sock, sendbuf, sending, 0);
		if(sent < 0){
			perror("[-]Error in sending message");
			return -1;
		}
#ifdef DEBUG
		printf("[+]Sent %i bytes of %i\n", sent, bytestosend);
#endif
		bzero(sendbuf, SENDSIZE);
		bufpos += sent;
		bytestosend -= sent;
		bufferroom = SENDSIZE;
	}
	printf("[+]Successfully sent entire message\n");
	return 0;
}

int send_msg(msg_t msg, conn_t conn){ // TODO might need to use protobuff for message layout
	if(IS_FILE_MSG(msg.type)){
		return sendfile_msg(msg, conn);
	}
	else{
		return sendupdt_msg(msg, conn);
	}
}


/*
 * The goal of this function is to abstract out some of the
 * error handling code from recv_msg. What is necessary to do
 * when handling the errors is different based on message type
 * and when the failure happens. This functions arguments will
 * also differ depending on when the error happens
 */
#define EINITMSG	(1<<0) // Error on receiving INIT MSG
#define EAOPEN		(1<<1) // Error After OPEN
#define EAALLOC		(1<<2) // Error After memory ALLOC (could be mmap or malloc, depends on if file or msg)
#define TFILE		(1<<3) // Type of message is FILE
#define TUPDT		(1<<4) // Type of message is UPDaTe
#define TUEMT		(1<<5) // Type of message is Update but the message is EMpTy
static msg_t recv_handleerror(msg_t in, char* pe, int flags){
	if(flags == EINITMSG)
		goto EXIT;

	if(flags & EAOPEN)
		close(in.type);

	if(flags & EAALLOC){
		if(flags & TUPDT)
			free(in.buf);
		else if(flags & TUEMT)
			goto EXIT;
		else if(flags & TFILE){
			munmap(in.buf, in.size);
		}
	}

EXIT:
	perror(pe);
	msg_t ret = {NULL, 0, NULL_MSG};
	return ret;
}

static msg_t recvfile_msg(conn_t conn, msg_t ret, int bytesread, char* prb){
	// prb => previous recv buffer
	int errint = TFILE, headersize = 256, fd, bytesleft;
	int pathstart = sizeof(int) + sizeof(size_t);
	char recvbuf[SENDSIZE], *bufpos;
	char name[256 - sizeof(int) - sizeof(size_t)] = {0};
	char relpath[sizeof(name)] = {0};
	memcpy(recvbuf, prb, bytesread);
	memcpy(name, recvbuf+pathstart, 256-pathstart);
	sprintf(relpath, "./%s", name);
	fd = open(relpath, O_RDWR | O_CREAT, 0644);
	if(fd < 0)
		return recv_handleerror(ret, "[-]Error creating temp file for file msg", errint);
	errint |= EAOPEN;

	ret.type = fd;
	if(ftruncate(fd, ret.size) < 0)
		return recv_handleerror(ret, "[-]Error increasing size of tmp in recv_msg", errint);

	ret.buf = mmap(NULL, ret.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(ret.buf == MAP_FAILED)
		return recv_handleerror(ret, "[-]Error on mmap in creating file for recv", errint);
	errint |= EAALLOC;

	bytesleft = (ret.size + headersize) - bytesread;
	// copy bytes read after the header into ret buffer
	memcpy(ret.buf, recvbuf+headersize, bytesread-headersize);
	bufpos = ret.buf + (bytesread - headersize);
	bzero(recvbuf, SENDSIZE);
	while(bytesleft > 0){
		bytesread = recv(conn.sock, recvbuf, SENDSIZE, 0);

		// error handling if recv fails
		if(bytesread < 0)
			return recv_handleerror(ret, "[-] Error in receiving part of message", errint);

		// copy bytes read into ret buf
		memcpy(bufpos, recvbuf, bytesread);
		bzero(recvbuf, SENDSIZE);
		bufpos += bytesread;
		bytesleft -= bytesread;
#ifdef DEBUG
		printf("[+]Successfully read and copied %i bytes, %i bytes left\n", bytesread, bytesleft);
#endif
	}
#ifdef DEBUG
	printf("[+]Successfully read entire message\n");
#endif
	return ret;

}

static msg_t recvupdt_msg(conn_t conn, msg_t ret, int bytesread){
	int errint = 0, headersize = sizeof(size_t) + sizeof(int), bytesleft;
	char recvbuf[SENDSIZE], *bufpos;
	if(ret.size == 0){
		ret.buf = NULL;
		errint |= TUEMT;
		return ret;
	}
	else{
		ret.buf = (char*)malloc(ret.size);
		if(ret.buf == NULL)
			return recv_handleerror(ret, "[-]Error on malloc for update message", errint);
		errint |= TUPDT;
	}

	bytesleft = (ret.size + headersize) - bytesread;
	// copy bytes read after the header into ret buffer
	memcpy(ret.buf, recvbuf+headersize, bytesread-headersize);
	bufpos = ret.buf + (bytesread - headersize);
	bzero(recvbuf, SENDSIZE);
	while(bytesleft > 0){
		bytesread = recv(conn.sock, recvbuf, SENDSIZE, 0);

		// error handling if recv fails
		if(bytesread < 0)
			return recv_handleerror(ret, "[-] Error in receiving part of message", errint);

		// copy bytes read into ret buf
		memcpy(bufpos, recvbuf, bytesread);
		bzero(recvbuf, SENDSIZE);
		bufpos += bytesread;
		bytesleft -= bytesread;
#ifdef DEBUG
		printf("[+]Successfully read and copied %i bytes, %i bytes left\n", bytesread, bytesleft);
#endif
	}
#ifdef DEBUG
	printf("[+]Successfully read entire message\n");
#endif
	return ret;
}

msg_t recv_msg(conn_t conn){
	int bytesread, bytesleft, fd;
	msg_t ret;
	char recvbuf[SENDSIZE] = {0}, *bufpos;

	// read initial message
	bytesread = recv(conn.sock, recvbuf, SENDSIZE, 0);
	printf("FJHLSDKFJLKSDFGJKLFJDL:🍔🍔🍔");
	if(bytesread < 0)
		return recv_handleerror(ret, "[-]Error in receiving initial part of message", EINITMSG);

	// parse type and size of msg_t from the first message
	ret.type = ((int*)recvbuf)[0];
	ret.size = ((size_t*)(recvbuf+sizeof(int)))[0];
#ifdef DEBUG
	printf("[+]Received message of type %d and size %lu\n", ret.type, ret.size);
#endif

	// allocate memory for message depending on type of message
	if(IS_FILE_MSG(ret.type)){
		return recvfile_msg(conn, ret, bytesread, recvbuf);
	}
	else{
		return recvupdt_msg(conn, ret, bytesread);
	}
}
