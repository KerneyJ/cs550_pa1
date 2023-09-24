#include <bits/types/FILE.h>
#include <cstdio>
#include <queue>
#include <string>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <vector>
#include "server.hpp"
#include "file_index.hpp"

extern "C" {
	#include "comms.h"
}

#include "thread_pool.hpp"

#define REPLICATION_FACTOR 2

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

	// TODO: error handling
    servinit_conn(&server_conn, ip, port);
	// TODO: error handling
	servlstn_conn(&server_conn, 5);
    
	servloop_conn(&server_conn, &message_handler, &stop);

	printf("Shutting down!\n");
	close_conn(&server_conn);

	return 0;
}

void register_user(conn_t client, msg_t message);
void register_file(conn_t client, msg_t message);
void search_index(conn_t client, msg_t message);
void request_replication(std::string filename);

FileIndex file_index;
std::vector<conn_t> peers;
std::queue<std::string> replication_queue;

void register_user(conn_t client, msg_t message) {
	peers.push_back(client);
}

void register_file(conn_t client, msg_t message) {
	std::string filename = message.buf;

	file_index.add_peer(filename, client);

	if(file_index.count_peers(filename) < REPLICATION_FACTOR) {
		request_replication(filename);
	}
}

void request_replication(std::string filename) {
	//TODO: implement
}

// - register(ip, file)
//     index.addUserIfNew(ip)
//     index.registerFileWUser(ip, file)
//     if(index.count(file) < Rep):            #Rep := replication number.
//         rep_req(random_ip, file, ip)
//         if(nobody else):
//             requeue.add(ip, file)
//     if(reqpqueue.notempty()):
//         rep_file, rep_ip = repqueue.pop()
//         rep-req(ip, rep_file, rep_ip)