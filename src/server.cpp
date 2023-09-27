
#include <memory>
#include <signal.h>
#include <cstdio>
#include <thread>
#include <vector>
#include "server.hpp"

static void connection_handler(conn_t client_conn, void message_handler(conn_t, msg_t)) {
	unsigned char* ip = (unsigned char*) &client_conn.addr;

	printf("Waiting for messages on my new connection: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], client_conn.port);
    msg_t message = recv_msg(client_conn);
    printf("💌💌📫 message.type %d, message.size %d, message.buf %s\n", message.type, message.size, message.buf);
    
    message_handler(client_conn, message);

	close_conn(&client_conn);
}

void servloop_conn(conn_t* server_conn, void (*message_handler)(conn_t, msg_t), volatile sig_atomic_t* interrupt) {
    ThreadPool threadpool = ThreadPool();

    conn_t client_conn;

    while(!(*interrupt)) {
        client_conn = servacpt_conn(server_conn);

        if(client_conn.addr == -1)
            continue;

        // threadpool.queue_job([client_conn, message_handler] {
            connection_handler(client_conn, message_handler);
        // });
    }

    threadpool.teardown();
}
