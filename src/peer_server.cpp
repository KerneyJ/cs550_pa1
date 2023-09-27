#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <mutex>
#include <set>
#include "server.hpp"
#include "peer.hpp"
#include "constants.hpp"

extern "C" {
	#include "comms.h"
}

static volatile sig_atomic_t stop;
static conn_t server_conn;

static void set_stop_flag(int signum) {
	printf("Catching SIGINT!\n");
    stop = 1;
	close_conn(&server_conn); // exits blocking accept() calls
}

void message_handler(conn_t client_conn, msg_t msg) {
	unsigned char* ip = (unsigned char*) &client_conn.addr;
	printf("Received a message of type %d\n", msg.type);

	if (msg.type == REQUEST_FILE) {
		send_file(client_conn, msg);
	} else if (msg.type == REPLICATION_REQ) {
		replicate_file(client_conn, msg);
	}
	// else if (msg.type == REQUEST_FILE) 
}

//Main server loop. Initiated from CLI. 
int main(int argc, char** argv) {
    signal(SIGINT, set_stop_flag);

    char *ip = "127.0.0.1";
    int port = 8081;

	// register_as_new_user();

    if(servinit_conn(&server_conn, ip, port) < 0) {
		printf("Failed to initialize server, shutting down.\n");
		return -1;
	}
	printf("ip=%d, port=%d\n", server_conn.addr, server_conn.port);

	if(servlstn_conn(&server_conn, 5)) {
		printf("Failed to start listening, shutting down.\n");
		return -1;
	}

	servloop_conn(&server_conn, message_handler, &stop);

	printf("Shutting down!\n");
	close_conn(&server_conn);

	return 0;
}
