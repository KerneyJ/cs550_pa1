
#include <memory>
#include <signal.h>
#include <cstdio>
#include <thread>
#include <string>
#include <vector>
#include "server.hpp"

static void connection_handler(conn_t client_conn, void message_handler(conn_t, msg_t)) {
	unsigned char* ip = (unsigned char*) &client_conn.addr;
#ifdef DEBUG
	printf("Waiting for messages on my new connection: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], client_conn.port);
#endif
    msg_t message = recv_msg(client_conn);
    
    message_handler(client_conn, message);

    delete_msg(&message);
	close_conn(&client_conn);
}

void servloop_conn(conn_t* server_conn, void (*message_handler)(conn_t, msg_t), volatile sig_atomic_t* interrupt) {
    ThreadPool threadpool = ThreadPool();
    conn_t client_conn;
#ifdef DEBUG
    printf("Starting server, accepting incoming connections!\n");
#endif
    while(!(*interrupt)) {
        client_conn = servacpt_conn(server_conn);

        if(client_conn.addr == -1)
            continue;

        threadpool.queue_job([client_conn, message_handler] {
            connection_handler(client_conn, message_handler);
        });
    }

    threadpool.teardown();
}

int parse_conn_arg(int argc, char** argv, int idx, conn_t* conn) {
	std::string ip, port;
	int split_idx;

	if(argc < idx + 1)
		return -1;

	std::string full_route(argv[1]);
	split_idx = full_route.find(':');

	if(split_idx == std::string::npos)
		return -1;

	ip = full_route.substr(0, split_idx);
	port = full_route.substr(split_idx + 1, full_route.size() - split_idx);

	conn->addr = inet_addr(ip.data());
	conn->port = stoi(port);

	return 0;
}