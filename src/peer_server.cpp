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
#ifdef DEBUG
	printf("Received a message of type %d\n", msg.type);
#endif
	if (msg.type == REQUEST_FILE) {
		send_file(client_conn, msg);
	} else if (msg.type == REPLICATION_REQ) {
		replicate_file(client_conn, msg);
	} else {
		printf("I don't know what to do with message type %d\n", msg.type);
	}
}

//Main server loop. Initiated from CLI. 
int main(int argc, char** argv) {
	signal(SIGINT, set_stop_flag);

	conn_t peer_server;
	conn_t index_server;

	if (parse_conn_arg(argc, argv, 1, &index_server) < 0 ||
		parse_conn_arg(argc, argv, 2, &peer_server) < 0) {
		printf("Please provide the ip and port that the index server is running on followed by the ip and port that this server will run on.\n");
		printf("\teg: ./peer_cli 185.236.36.234:8080 127.0.0.1:8080\n");
		printf("\t       	    index server ⤴     peer server ⤴   \n");
		return -1;
	}

	set_index_server_conn(index_server);
	set_peer_server_conn(peer_server);

#ifdef DEBUG
	printf("Registering with the index server.\n");
#endif

	if(servinitco_conn(&server_conn, &peer_server) < 0) {
		printf("Failed to initialize server, shutting down.\n");
		return -1;
	}
#ifdef DEBUG
	printf("ip=%d, port=%d\n", server_conn.addr, server_conn.port);
#endif

	register_as_new_user();
	if(servlstn_conn(&server_conn, 5)) {
		printf("Failed to start listening, shutting down.\n");
		return -1;
	}

	servloop_conn(&server_conn, message_handler, &stop);

	printf("Shutting down!\n");
	close_conn(&server_conn);

	return 0;
}
