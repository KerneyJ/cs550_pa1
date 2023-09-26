/* Booted up by peer_CLI*/

/*
Every time you message server, you use the msg_t struct. You need to choose the 
msg_type appropriately depending on the message so the receiving host knows 
how to handle the message.

Running a server loop with server.cpp

*/

#include "peer.hpp" // includes stdlib.h string.h
#include "comms.h"
#include "thread_pool.hpp"
#include <stdio.h>
#include "comms.h"
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "constants.hpp" 

#define IP_LENGTH 24
#define MAX_DIR_NAME_SIZE 1024

char index_server_ip[IP_LENGTH];
char local_shared_dir[MAX_DIR_NAME_SIZE];

//Given an ip of the destination host, the msg_type, and the message content (buf), send a message using the comms.c interface.
msg_type send_msg_wrapper(char *ip, int msg_type, char *buf) {
	//TODO DELETE
	return NULL_MSG;
}

//Connect as new user. Conect to index server (which will get my ip) and register_dir(directory).
int register_as_new_user() {
	msg_t message;
	conn_t client;

	create_message(&message, "", NEW_USER);

	clntinit_conn(&client, INDEX_SERVER_IP, INDEX_SERVER_PORT);
	send_msg(message, client);
	close_conn(&client);

	delete_msg(&message);

	return 0;
}




/*Registers all the files in a directory. Just calls register file for every file in dir*/
int register_dir(char* dirname) {
	DIR *dir;
	struct dirent *entry;
	int files_registered = 0;
	dir = opendir(dirname);

	if (dir == NULL) {
		perror("opendir");
		return 1;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_REG) {
			if (register_file(entry->d_name) < 0) {
				closedir(dir);
				return -1; //error registering file
			}
			files_registered++;
		}
	}

	closedir(dir);
	return files_registered;
}

//Sends a file to a host. Expects an ip address char array of the receiving host, and a name of file char array of the file to be sent. Returns 0 if successful.
int send_file(conn_t client_conn, msg_t message) {
	//This is called when the host server receives a msg_t with msg_type == REQUEST_FILE.
	//The host then sends the message to the IP address of the host requesting the file by using the comms.c interface. 
	msg_t file_message;
	char* path;
	
	sprintf(path, "%s/%s", SHARED_FILE_DIR, message.buf);

	if(createfile_msg(&file_message, path) < 0) {
		msg_t err_msg;
		create_message(&err_msg, "", STATUS_BAD);
		send_msg(err_msg, client_conn);
		delete_msg(&err_msg);
		perror("File could not be sent!\n");
		return -1;
	}

	send_msg(file_message, client_conn);
	delete_msg(&file_message);

	printf("File successfully sent!\n");

	return 0;
}

//Request a file from peer, given the ip address of the peer who has the file and the name of the file being requested.
int request_file_from_peer(conn_t peer, char* filename) {
	//Send a msg_t to the ip of the file owner host with msg_type == -3 and buf == filename.
	//If the host responds by sending you the file, save the file to disk at local_shared_dir.
	//optional: If the host does not respond, query the index_server for an alternative host until there are no more hosts left to try or you download the file successfuly.
	conn_t client_conn;
	msg_t req, res;

	// temp solution
	char ip_str[80];
	unsigned char* ip = (unsigned char*) &peer.addr;
	sprintf(ip_str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	clntinit_conn(&client_conn, ip_str, peer.port);
	
	// clntinit_conn(&client_conn, &peer); // Jamie todo
	create_message(&req, filename, REQUEST_FILE);
	send_msg(req, peer);
	delete_msg(&req);

	res = recv_msg(client_conn);

	if(res.type == NULL_MSG) {
		close_conn(&client_conn);
		perror("Message failed to receive.\n");
		return -1;
	}

	if(res.type == STATUS_BAD) {
		close_conn(&client_conn);
		perror("Peer did not have requested file.\n");
		return -1;
	}

	printf("File successfully downloaded!\n");

	close_conn(&client_conn);
	delete_msg(&res);

	//TODO: register file after receiving.
	// register_file(filename);

	return 0;
}

//expects the ip address of the host who has the file (char*), and the name of the file to replicate (char*). Returns 0 if successful.
int replicate_file(conn_t client_conn, msg_t message) {
	return request_file_from_peer(client_conn, message.buf);
}

/*Registers a file with the index server. Sends name_of_file to index server. Doesn't wait for a response.*/
int register_file(char* name_of_file) {
	msg_t message;
	conn_t connection;
	create_message(&message, name_of_file, REGISTER_FILE);
	clntinit_conn(&connection,INDEX_SERVER_IP, INDEX_SERVER_PORT);
	if (send_msg(message, connection) < 0) {
		delete_msg(&message);
		close_conn(&connection);
		return -1;
	}
	delete_msg(&message);
	close_conn(&connection);

	return 0;
}

//Searches for a file on the index server. Expects a filename. Returns the ip address of a host who owns the file if found.
conn_t search_for_file(char* filename) {
	
	msg_t message;
	conn_t connection;
	msg_t reply;
	create_message(&message, filename, SEARCH_INDEX);
	clntinit_conn(&connection,INDEX_SERVER_IP, INDEX_SERVER_PORT);
	send_msg(message, connection);
	delete_msg(&message);
	reply = recv_msg(connection);
	delete_msg(&reply);
	close_conn(&connection);
	if (reply.type == STATUS_BAD) {
		return {-1,-1,-1}; //Sorry no Dave Grohl.

	}
	//Otherwise the file exists.
	int *reply_data = (int*)reply.buf;
	int host_ip = reply_data[0];
	int host_port = reply_data[1];
	return {host_ip, host_port, 0};
}

//Download a file. Expects filename. Asks index server for ip address of host with file. Then asks host for the file. Returns 0 if successful.
//This function just wraps the search_for_file and request_file_from_peer functions to make for a simple call from the CLI.
// int request_file(char* filename) {
// 	conn_t file_owner_ip = search_for_file(filename);
// 	//If the file exists, request it from the peer host. 
// 	if (file_owner_ip.addr != -1) {
// 		if (request_file_from_peer(file_owner_ip.addr, filename)) {
// 			return -2; //Peer didn't have file. 
// 		}
// 		//Successful file download. Register file with index server.
// 		else {
// 			if(register_file(filename)) {
// 				return -3; //Failed to register file with server.
// 			}
// 			else {
// 				return 0;
// 			}
// 		}
// 	}
// 	else {
// 		return -1; //File doesn't exist
// 	}
// }