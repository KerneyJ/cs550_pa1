#include <stdio.h>
#include "comms.h"

int main(char** argv, int argc){
	if(fork()){
		conn_t conn = {0, 0, 0};
		servinit_conn(&conn, "127.0.0.1", 8080);
		servlstn_conn(&conn, 5);
		conn_t client = servacpt_conn(&conn);
		close_conn(&conn);
		close_conn(&client);
	}
	else{
		sleep(3);
		conn_t conn = {0, 0, 0};
		clntinit_conn(&conn, "127.0.0.1", 8080);
		close_conn(&conn);
	}
	return 0;
}
