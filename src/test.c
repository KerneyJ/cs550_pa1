#include <stdio.h>
#include "comms.h"

int main(char** argv, int argc){
	if(fork()){
		char recvbuf[SENDSIZE] = {0};
		conn_t conn = {0, 0, 0};
		servinit_conn(&conn, "127.0.0.1", 8080);
		servlstn_conn(&conn, 5);
		conn_t client = servacpt_conn(&conn);
		recv_msg(client, recvbuf);
		size_t size;
		int type, bytesread = recv(conn.sock, recvbuf, SENDSIZE, 0);
		type = ((int*)recvbuf)[0];
		size = ((size_t*)(recvbuf+sizeof(int)))[0];
		printf("[+]Received message of type %d and size %lu\n", type, size);
		close_conn(&conn);
		close_conn(&client);
	}
	else{
		sleep(3);
		conn_t conn = {0, 0, 0};
		clntinit_conn(&conn, "127.0.0.1", 8080);
		msg_t msg;
		createupdt_msg(&msg, "Hello server", 12);
		send_msg(msg, conn);
		close_conn(&conn);
	}
	return 0;
}
