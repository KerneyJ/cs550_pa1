#include "server.hpp"
#include "comms.h"

int register_as_new_user(char*); //expects a char array of a directory path. Returns 0 if successful.
int register_file(char*); //Expects char array of the file name to be registered. Returns 0 if successful.
int register_dir(char*); //expects a char array of a directory path. Returns 0 if successful.
int send_file(char*, char*);  //expects an ip address char array of the receiving host, and a name of file char array of the file to be sent. Returns 0 if successful.
int replicate_file(char*, char*); //expects the ip address of the host who has the file (char*), and the name of the file to replicate (char*). Returns 0 if successful.
