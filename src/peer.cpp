#include <stdio.h>

extern "C" {
	#include "comms.h"
}

int main(int argc, char** argv) {
	conn_t conn;
	clntinit_conn(&conn, "127.0.0.1", 8080);
	close_conn(&conn);
	return 0;
}
