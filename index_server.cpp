#include "stdio.h"
#include <sys/socket.h>
#include <thread>
#include <signal.h>

extern "C" {
	#include "comms.h"
}

volatile sig_atomic_t stop;
conn_t server_conn;

void set_stop_flag(int signum) {
    stop = 1;
	close_conn(&server_conn); // exits blocking accept() calls
}

void handle_connection(conn_t client_conn) {
	unsigned char* ip = (unsigned char*) &client_conn.addr;
	printf("Doing stuff with my new connection: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], client_conn.port);
	close_conn(&client_conn);
}

int main(int argc, char** argv) {

    signal(SIGINT, set_stop_flag);

    char *ip = "127.0.0.1";
    int port = 8080;
    
    conn_t client_conn;

    servinit_conn(&server_conn, ip, port);
	servlstn_conn(&server_conn, 5);
    
    while(!stop) {
        client_conn = servacpt_conn(&server_conn);

		if(client_conn.addr == -1)
			continue;

		std::thread(handle_connection, client_conn).detach();
    }

	close_conn(&server_conn);
	printf("Shutting down!\n");

	return 0;
}
