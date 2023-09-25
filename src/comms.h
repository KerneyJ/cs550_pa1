#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#pragma once

#define UPDATE_MSG_SIZE 256
#define SENDSIZE 1024

#define IS_FILE_MSG(msg_type) msg_type > 0

enum msg_type {
	NULL_MSG		=  0,
	NEW_USER		= -1,
	SEARCH_INDEX	= -2,
	REQUEST_FILE	= -3,
	REGISTER_FILE	= -4,
	REPLICATION_REQ	= -5,
	STATUS_OK		= -6,
	STATUS_BAD		= -7,
};

typedef struct {
	int addr;
	int port;
	int sock;
} conn_t;

typedef struct {
	char* buf;
	size_t size;
	enum msg_type type;
} msg_t;

int servinit_conn(conn_t *, char*, int);
int servlstn_conn(conn_t *, int);
conn_t servacpt_conn(conn_t *);
int clntinit_conn(conn_t *, char*, int);
int close_conn(conn_t *);

int createupdt_msg(msg_t*, char*, int, int);
int createfile_msg(msg_t*, char*);
int delete_msg(msg_t*);

int send_msg(msg_t, conn_t);
/*
 * If you receive a file message you must immedieatly rename the file 
 * that is assocaited with the message
 */
msg_t recv_msg(conn_t);
