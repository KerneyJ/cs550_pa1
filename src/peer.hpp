#include "server.hpp"
extern "C" {
	#include "comms.h"
}


int register_as_new_user(char*, char*); //expects a char array of a directory path, and char array of the index server's ip. Returns 0 if successful.
int register_file(char*); //Expects char array of the file name to be registered. Returns 0 if successful.
int register_dir(char*); //expects a char array of a directory path. Returns 0 if successful.
int send_file(char*, char*);  //expects an ip address char array of the receiving host, and a name of file char array of the file to be sent. Returns 0 if successful.
int replicate_file(char*, char*); //expects the ip address of the host who has the file (char*), and the name of the file to replicate (char*). Returns 0 if successful.
char* search_for_file(char*); //Searches for a file on the index server. Expects a filename. Returns the ip address of a host who owns the file if found.
int request_file(char*); //Download a file. Expects filename. Asks index server for ip address of host with file. Then asks host for the file. Returns 0 if successful.
int run_peer_server(char*, char*); //Initiates the server on peer hosts. Expects the ip address of the index server, and the local shared directory name.