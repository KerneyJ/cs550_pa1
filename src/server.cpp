
#include <memory>
#include <signal.h>
#include <cstdio>
#include <thread>
#include <string>
#include <vector>
#include "server.hpp"

static conn_t init_new_connection() {
	struct sockaddr_in serv_addr;
    int ip, port, sock;

    if(get_ipv4_address(&ip) < 0)
        return {-1, -1, -1};
    
	sock = socket(AF_INET, SOCK_STREAM, 0);
    for(port = 8000; port < 9000; port++) {
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = port;
        serv_addr.sin_addr.s_addr = ip;

    	if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            // failed to bind, check next port
            continue;
        }

        close(sock);

        return {ip, port, -1};
    }

    return {-1, -1, -1};
}

void Server::connection_handler(conn_t client_conn, void message_handler(conn_t, msg_t)) {
	unsigned char* ip = (unsigned char*) &client_conn.addr;
#ifdef DEBUG
	printf("Waiting for messages on my new connection: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], client_conn.port);
#endif
    msg_t message = recv_msg(client_conn);
    
    message_handler(client_conn, message);

    delete_msg(&message);
	close_conn(&client_conn);
}

void Server::server_loop(ThreadPool* threadpool, conn_t* server_conn, void (*message_handler)(conn_t, msg_t), volatile sig_atomic_t *interrupt) {
	conn_t client_conn;

    printf("Starting server, accepting incoming connections!\n");
	while(!(*interrupt)) {
		client_conn = servacpt_conn(server_conn, interrupt);

		if(client_conn.addr == -1)
			continue;

		threadpool->queue_job([client_conn, message_handler] {
			connection_handler(client_conn, message_handler);
		});
	}
}

int Server::start(void (*message_handler)(conn_t, msg_t), bool blocking) {
    conn_t conn = init_new_connection();

    if(servinitco_conn(&local_server, &conn) < 0) {
		printf("Failed to initialize server, shutting down.\n");
		return -1;
	}

	if(servlstn_conn(&local_server, 32)) {
		printf("Failed to start listening, shutting down.\n");
		return -1;
	}
    
	if(blocking) {
	    server_loop(threads, &local_server, message_handler, &interrupt);
	} else {
		threads->queue_job([this, message_handler] {
			server_loop(threads, &local_server, message_handler, &interrupt);
		});
	}

    return -1;
}

Server::Server() {
    interrupt = 0;
    threads = new ThreadPool();
}

Server::~Server() {
	printf("Shutting down server.\n");

    close_conn(&local_server);
    interrupt = 1;
    delete threads;
}