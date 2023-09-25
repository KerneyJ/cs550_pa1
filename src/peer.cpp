/* Booted up by peer_CLI*/

/*
Every time you message server, you use the msg_t struct. You need to choose the 
msg_type appropriately depending on the message so the receiving host knows 
how to handle the message.

Running a server loop with server.cpp

*/

#include <stdio.h>




/*  
Connect as new user. Conect to index server (which will get my ip)
register_dir(directory)
 */
int register_as_new_user(char* dir_name){
	//TODO
}


/*Registers a file with the index server. Sends name_of_file to index server.*/
int register_file(char* name_of_file) {
	//TODO
}

/*Registers all the files in a directory. Just calls register file for every file in dir*/
int register_dir(char* dir_name) {
	//TODO
}

//Sends a file to a host. Expects an ip address char array of the receiving host, and a name of file char array of the file to be sent. Returns 0 if successful.
int send_file(char* ip, char* dir_name) {
	//TODO
}		

//expects the ip address of the host who has the file (char*), and the name of the file to replicate (char*). Returns 0 if successful.
int replicate_file(char* ip, char* filename) {
	//TODO
}

//Searches for a file on the index server. Expects a filename. Returns the ip address of a host who owns the file if found.
char* search_for_file(char* filename) {

}

//Download a file. Expects filename. Asks index server for ip address of host with file. Then asks host for the file. Returns 0 if successful.
int request_file(char* filename) {
	//TODO
}


//Main server loop. Initiated from CLI. 
int run_peer_server() {
	//TODO
}
