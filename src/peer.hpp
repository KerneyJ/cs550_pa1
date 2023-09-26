extern "C" {
    #include "comms.h"
}

#include "server.hpp" // this also includes comms.h


int register_as_new_user(); //expects a char array of a directory path, and char array of the index server's ip. Returns 0 if successful.
int register_file(char*); //Expects as input the file name to be registered. Returns 0 if successful.
int register_dir(char*); //expects a char array of a directory path. Returns 0 if successful.
int send_file(conn_t client_conn, msg_t message); //expects an ip address char array of the receiving host, and a name of file char array of the file to be sent. Returns 0 if successful.
int request_file_from_peer(conn_t peer, char* filename);
int replicate_file(conn_t, msg_t); //expects the ip address of the host who has the file (char*), and the name of the file to replicate (char*). Returns 0 if successful.
conn_t search_for_file(char*); //Searches for a file on the index server. Expects a filename. Returns the ip address of a host who owns the file if found.
int request_file(char*); //Download a file. Expects filename. Asks index server for ip address of host with file. Then asks host for the file. Returns 0 if successful.
int run_peer_server(char*, int, char*); //Initiates the server on peer hosts. Expects the ip address of the index server, and the local shared directory name.
void shutdown_peer();