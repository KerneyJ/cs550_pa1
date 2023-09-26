
#include <signal.h>
#include <cstdio>
#include <thread>
#include <vector>
#include "server.hpp"

void connection_handler(conn_t client_conn, void message_handler(conn_t, msg_t)) {
    msg_t msg;

	unsigned char* ip = (unsigned char*) &client_conn.addr;
	printf("Waiting for messages on my new connection: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], client_conn.port);

	close_conn(&client_conn);
}

void main_server_loop(ThreadPool* threadpool, conn_t* server_conn, void (*message_handler)(conn_t, msg_t), volatile sig_atomic_t* interrupt) {
    conn_t client_conn;

    while(!(*interrupt)) {
        // TODO: error handling
        client_conn = servacpt_conn(server_conn);

        if(client_conn.addr == -1)
            continue;

        threadpool->queue_job([client_conn, message_handler] {
            connection_handler(client_conn, message_handler);
        });
    }
}

void servloop_conn(ThreadPool* threadpool, conn_t* server_conn, void (*message_handler)(conn_t, msg_t), volatile sig_atomic_t* interrupt, bool blocking) {
    if(blocking) {
        main_server_loop(threadpool, server_conn, message_handler, interrupt);
    } else {
        threadpool->queue_job([threadpool, server_conn, message_handler, interrupt] {
            main_server_loop(threadpool, server_conn, message_handler, interrupt);
        });
    }
}

void servloop_conn(conn_t* server_conn, void (*message_handler)(conn_t, msg_t), volatile sig_atomic_t* interrupt) {
    ThreadPool threadpool = ThreadPool();

    servloop_conn(&threadpool, server_conn, message_handler, interrupt);

    threadpool.teardown();
}
