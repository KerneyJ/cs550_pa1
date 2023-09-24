#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define UPDATE_MSG_SIZE 256

#define SENDSIZE 1024

enum type{
	update=-1,
};

typedef struct {
	int addr;
	int port;
	int sock;
} conn_t;

typedef struct {
	char* buf;
	size_t size;
	int type;
} msg_t;

int servinit_conn(conn_t *, char*, int);
int servlstn_conn(conn_t *, int);
conn_t servacpt_conn(conn_t *);
int clntinit_conn(conn_t *, char*, int);
int close_conn(conn_t *);

int createupdt_msg(msg_t*, char*, int);
int createfile_msg(msg_t*, char*);
int delete_msg(msg_t*);

int send_msg(msg_t, conn_t);
int recv_msg(conn_t, char*);
