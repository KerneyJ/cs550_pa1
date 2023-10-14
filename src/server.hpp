#include <signal.h>
#include "thread_pool.hpp"

extern "C" {
	#include "comms.h"
}

#pragma once

class Server {
    private:
        ThreadPool* threads;
        volatile sig_atomic_t interrupt;
        conn_t local_server;
		static void connection_handler(conn_t, void (conn_t, msg_t));
		static void server_loop(ThreadPool*, conn_t*, void (*)(conn_t, msg_t), volatile sig_atomic_t*);
    public:
        Server();
        ~Server();
        int start(void (*)(conn_t, msg_t), bool blocking = true);
		conn_t get_conn_info();
};