#include <signal.h>
#include "thread_pool.hpp"

extern "C" {
	#include "comms.h"
}

#pragma once

void servloop_conn(ThreadPool* threadpool, conn_t* server_conn, void (*message_handler)(conn_t, msg_t), volatile sig_atomic_t* interrupt, bool blocking = true);
void servloop_conn(conn_t* server_conn, void (*message_handler)(conn_t, msg_t), volatile sig_atomic_t* interrupt);