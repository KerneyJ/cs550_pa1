#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#pragma once

#define SENDSIZE 1024

#define IS_FILE_MSG(msg_type) msg_type > 0

enum msg_type {
	FILE_MSG		=  1, // DO NOT COMPARE TO THIS ENUM VALUE!!!!! file messages can be of type positive integer
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

/*
 *	int create_message(msg_t*, char*, int);
 *		wrapper for createupdt_msg and createfile_msg
 *		Arguments
 *			msg_t* msg: pointer to a single message, pointer to write to
 *			char* string: could be a path or message string depending on type
 *			int type: for update message use enum msg_type; for file positive int
 *		Returns
 *			the output of the message creation function specified by type
 */
int create_message(msg_t*, char*, int);

/*
 * int createupdt_msg(msg_t*, char*, int, int);
 *		create a message that is not a file
 *		Arguments
 *			msg_t* msg: pointer to a single message, pointer to write to
 *			char* update_message: extra infomration to go with enum msg_type
 *			int len: length of char* update_message
 *			int type: enum msg_typ
 *		Returns
 *			-1 on error, perror is called
 *			0 on success
 */
int createupdt_msg(msg_t*, char*, int, int);

/*
 * int createfile_msg(msg_t*, char*);
 *		create a message that is not a file
 *		Arguments
 *			msg_t* msg: pointer to a single message, pointer to write to
 *			char* path: path of file to create message out of
 *		Returns
 *			-1 on error, perror is called
 *			0 on success
 */
int createfile_msg(msg_t*, char*);

/*
 * int delete_msg(msg_t*);
 *		create a message that is not a file
 *		Arguments
 *			msg_t* msg: pointer to a single message, pointer to change
 *		Returns
 *			-1 on error, perror is called
 *			0 on success
 */
int delete_msg(msg_t*);

int send_msg(msg_t, conn_t);
/*
 * If you receive a file message you must immedieatly rename the file 
 * that is assocaited with the message
 */
msg_t recv_msg(conn_t);
