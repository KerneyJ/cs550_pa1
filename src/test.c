#include <stdio.h>
#include "comms.h"

int main(char** argv, int argc){
	if(fork()){
		char recvbuf[SENDSIZE] = {0};
		conn_t conn = {0, 0, 0};
		servinit_conn(&conn, "127.0.0.1", 8080);
		servlstn_conn(&conn, 5);
		conn_t client = servacpt_conn(&conn);
		msg_t msg = recv_msg(client);
		printf("[+]Received message of type %d and size %lu\n", msg.type, msg.size);
		close_conn(&conn);
		close_conn(&client);
	}
	else{
		sleep(3);
		conn_t conn = {0, 0, 0};
		clntinit_conn(&conn, "127.0.0.1", 8080);
		msg_t msg;
		createfile_msg(&msg, "./vm0_1M_000000.data");
		//createupdt_msg(&msg, "Hello server", 12, NEW_USER);
		send_msg(msg, conn);
		delete_msg(&msg);
		close_conn(&conn);
	}
	return 0;
}
