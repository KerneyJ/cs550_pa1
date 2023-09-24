#include <cstdio>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include "server.hpp"

extern "C" {
	#include "comms.h"
}

#include "thread_pool.hpp"

volatile sig_atomic_t stop;
conn_t server_conn;

void set_stop_flag(int signum) {
	printf("Catching SIGINT!\n");
    stop = 1;
	close_conn(&server_conn); // exits blocking accept() calls
}

void message_handler(conn_t client_conn, msg_t msg) {
	unsigned char* ip = (unsigned char*) &client_conn.addr;
	printf("Doing stuff with my new connection: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], client_conn.port);
}

int main(int argc, char** argv) {

    signal(SIGINT, set_stop_flag);

    char *ip = "127.0.0.1";
    int port = 8080;

    conn_t client_conn;

    servinit_conn(&server_conn, ip, port);
	servlstn_conn(&server_conn, 5);
    
	servloop_conn(&server_conn, &message_handler, &stop);

	printf("Shutting down!\n");
	close_conn(&server_conn);

	return 0;
}
