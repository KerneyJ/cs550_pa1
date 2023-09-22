
#include <bits/types/sig_atomic_t.h>
#include <cstdio>
#include <thread>
#include <vector>
#include "server.hpp"
#include "thread_pool.hpp"

void connection_handler(conn_t client_conn, void message_handler(conn_t, msg_t)) {
    msg_t msg;

	unsigned char* ip = (unsigned char*) &client_conn.addr;
	printf("Waiting for messages on my new connection: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], client_conn.port);

	close_conn(&client_conn);
}

void servloop_conn(conn_t* server_conn, void (*message_handler)(conn_t, msg_t), volatile sig_atomic_t* interrupt) {
    conn_t client_conn;

    ThreadPool threadpool = ThreadPool();

    while(!(*interrupt)) {
        client_conn = servacpt_conn(server_conn);

		if(client_conn.addr == -1)
			continue;

        threadpool.queue_job([client_conn, message_handler] {
            connection_handler(client_conn, message_handler);
        });
		// std::thread(connection_handler, client_conn, message_handler).detach();
    }
}
