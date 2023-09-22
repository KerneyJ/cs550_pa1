#include <signal.h>

extern "C" {
	#include "comms.h"
}

void servloop_conn(conn_t* server_conn, void (*message_handler)(conn_t, msg_t), volatile sig_atomic_t* interrupt);