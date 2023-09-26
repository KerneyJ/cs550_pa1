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
#include <dirent.h>

#define IP_LENGTH 24
#define MAX_DIR_NAME_SIZE 1024

static volatile sig_atomic_t stop;
static conn_t server_conn;
static ThreadPool threadpool;

char index_server_ip[IP_LENGTH];
char local_shared_dir[MAX_DIR_NAME_SIZE];

//Given an ip of the destination host, the msg_type, and the message content (buf), send a message using the comms.c interface.
msg_type send_msg_wrapper(char *ip, int msg_type, char *buf) {
	//TODO DELETE
	return NULL_MSG;
}

//Connect as new user. Conect to index server (which will get my ip) and register_dir(directory).
int register_as_new_user(char* dir_name, char* index_ip){
	strcpy(index_server_ip, index_ip);
	strcpy(local_shared_dir, dir_name);

	if (register_dir(dir_name)) {
		return 1; //error registering directory.
	}
	return 0;
}


/*Registers a file with the index server. Sends name_of_file to index server.*/
int register_file(char* name_of_file) {
	int msg_status = send_msg_wrapper(index_server_ip, REGISTER_FILE, name_of_file);
	if (msg_status == -7) {
		return 1; //-7 means there was an error.
	}
	printf("Registered file: %s", name_of_file);
	return 0;
}

/*Registers all the files in a directory. Just calls register file for every file in dir*/
int register_dir(char* dirname) {
	DIR *dir;
	struct dirent *entry;

	dir = opendir(dirname);

	if (dir == NULL) {
		perror("opendir");
		return 1;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_REG) {
			if (register_file(entry->d_name)) {
				return 1; //error registering file
			};
		}
	}

	closedir(dir);
	return 0;
}

//Sends a file to a host. Expects an ip address char array of the receiving host, and a name of file char array of the file to be sent. Returns 0 if successful.
int send_file(char* ip, char* filename) {
	//TODO

	//This is called when the host server receives a msg_t with msg_type == REQUEST_FILE.
	//The host then sends the message to the IP address of the host requesting the file by using the comms.c interface. 
	msg_type msg_status = send_msg_wrapper(ip, FILE_MSG, filename);
	if (msg_status == -7) {
		return 1; //Message error
	}
	return 0;
}

//Request a file from peer, given the ip address of the peer who has the file and the name of the file being requested.
int request_file_from_peer(char* ip, char* filename) {
	//TODO
	//Send a msg_t to the ip of the file owner host with msg_type == -3 and buf == filename.
	//If the host responds by sending you the file, save the file to disk at local_shared_dir.
	//optional: If the host does not respond, query the index_server for an alternative host until there are no more hosts left to try or you download the file successfuly.
	send_msg_wrapper(ip, REQUEST_FILE, filename);
}

//expects the ip address of the host who has the file (char*), and the name of the file to replicate (char*). Returns 0 if successful.
int replicate_file(char* ip, char* filename) {
	//TODO
	return request_file_from_peer(ip, filename);
}

//Searches for a file on the index server. Expects a filename. Returns the ip address of a host who owns the file if found.
char* search_for_file(char* filename) {
	//TODO
	//Send a msg_t to index_server with msg_type == -2, and buf==filename.
	//Then if the file exists, return the ip address of the host who has the file.
	send_msg_wrapper(index_server_ip, SEARCH_INDEX, filename);
}

//Download a file. Expects filename. Asks index server for ip address of host with file. Then asks host for the file. Returns 0 if successful.
//This function just wraps the search_for_file and request_file_from_peer functions to make for a simple call from the CLI.
int request_file(char* filename) {
	char *file_owner_ip = search_for_file(filename);
	//If the file exists, request it from the peer host. 
	if (file_owner_ip) {
		if (request_file_from_peer(file_owner_ip, filename)) {
			return -2; //Peer didn't have file. 
		}
		//Successful file download. Register file with index server.
		else {
			if(register_file(filename)) {
				return -3; //Failed to register file with server.
			}
			else {
				return 0;
			}
		}
	}
	else {
		return -1; //File doesn't exist
	}
}

void message_handler(conn_t client_conn, msg_t msg) {
	unsigned char* ip = (unsigned char*) &client_conn.addr;
	printf("Doing stuff with my new connection: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], client_conn.port);
}

static void set_stop_flag(int signum) {
	printf("Catching SIGINT!\n");
	shutdown_peer();
}

void shutdown_peer() {
    stop = 1;
	close_conn(&server_conn); // exits blocking accept() calls
	threadpool.teardown();
}

//Main server loop. Initiated from CLI. 
int run_peer_server(char *index_server_ip, int index_server_port, char* dir_name) {
    signal(SIGINT, set_stop_flag);

	register_as_new_user(dir_name, index_server_ip);

    char *ip = "127.0.0.1";
    int port = 8081;

    if(servinit_conn(&server_conn, ip, port) < 0) {
		printf("Failed to initialize server, shutting down.\n");
		return -1;
	}

	if(servlstn_conn(&server_conn, 5)) {
		printf("Failed to start listening, shutting down.\n");
		return -1;
	}

	servloop_conn(&threadpool, &server_conn, message_handler, &stop, false);

	return 0;
}
