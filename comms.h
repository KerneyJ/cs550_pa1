#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

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

int servinit_conn(conn_t *);
int servlstn_conn(conn_t *);
conn_t servacpt_conn(conn_t *);
int clntinit_conn(connt_t *);
int close_conn(conn_t *);

int send_msg(msg_t); // TODO
int recv_msg(); // TODO
