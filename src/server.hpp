#include <signal.h>
#include "thread_pool.hpp"

extern "C" {
	#include "comms.h"
}

#pragma once

typedef std::function<void(conn_t, msg_t)> msg_func;

class Server {
    private:
        ThreadPool* threads;
        volatile sig_atomic_t interrupt;
        conn_t local_server;
		static void connection_handler(conn_t, msg_func);
		static void server_loop(ThreadPool*, conn_t*, msg_func, volatile sig_atomic_t*);
    public:
        Server(bool use_fixed_port = true);
        ~Server();
        int start(msg_func message_handler, bool blocking = true);
		conn_t get_conn_info();
};