/* Booted up by peer_CLI*/


#include <stdio.h>

extern "C" {
	#include "comms.h"
}

// int main(int argc, char** argv) {
// 	conn_t conn;
// 	clntinit_conn(&conn, "127.0.0.1", 8080);
// 	close_conn(&conn);
// 	return 0;
// }

/*
Every time you message server, you use the msg_t struct. You need to choose the 
msg_type appropriately depending on the message so the receiving host knows 
how to handle the message.

Running a server loop with server.cpp

*/



//Register_as_a_new_user(user_ip, directory)
/*  conect to index server (which will get my ip)
	register_dir(directory)
 */


//register_file(name_of_file)
/*Send name_of_file to index server.*/


//Register dir
/*Just calls register file for every file in dir*/

//Search_index(filename)
//Expects a reply from index server. Returns ip of host with file if it exists, else -1


//Request_file(file_to_downlooad)
/*sends to server name of file to download.
	Server responds with a ip address of host with file.
	Then sends file download request to peer IP.
	Then the peer 
*/

//send_file 		(Send a file to a peer who requests it)

//replication_request(file_to_dup, ip)
/* requests download of file_to_dup from ip

*/
