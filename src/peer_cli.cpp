#include <stdio.h> 
#include "peer.hpp"
#include "constants.hpp"

#define MAX_DIR_NAME_SIZE 1024


int launch_CLI() {

	char *ip = INDEX_SERVER_IP;
	int port = INDEX_SERVER_PORT;
	char *file_dir = SHARED_FILE_DIR;

	//Welcome
	printf("\n-------------------------------------------------\n");
	printf("Howdy partner 🤠, ready to pirate some 💿💿💿?");
	printf("\n-------------------------------------------------\n\n");

	int is_running = 1;
	int user_input;
	char local_shared_dir[MAX_DIR_NAME_SIZE];
	while (is_running) {
		//Main menu

		printf("\nSelect a menu item:\n");
		printf("--------------------\n");
		printf("[1] 🤝 Register as a new user (and file directory)\n");
		printf("[2] 💰 Register a file\n");
		printf("[3] 🔎 Search for a file on the server index\n");
		printf("[4] 📦 Request a file to download from the network\n");
		printf("[9] 👋 quit (and log off the network).\n");

		scanf("%d", &user_input);
		if (user_input == 9) {
			printf("Aye Matey, Farewell! 👋\n\n");
			is_running = 0;
		}
		else if (user_input == 1) {
			printf("\nRegistering new user...\n");
			printf("Enter the directory where you will be sharing and downloading files: ");
			printf("Enter a directory name (up to %d characters): ", MAX_DIR_NAME_SIZE - 1);
			scanf("%99s", local_shared_dir);
			
			// register_file()

			printf("You entered: %s\n", local_shared_dir);
		}
		else if (user_input == 2) {
			printf("Registering a new file...\n");
		}
		else if (user_input == 3) {
			printf("Searching for a file on the server...\n");
			//read in the filename from user
			char *filename = "hithereimafile"; //TODO
			conn_t reply = search_for_file(filename);
			if (reply.addr == -1) {
				printf("file doesn't exist.");
			}
			else {
				printf("File found on host at IP {%d}, port {%d}", reply.addr, reply.port);
			}
			
		}
		else if (user_input ==4) {
			printf("Requesting a file to download...\n");
		}
	}
	return 0;
}



int main(int argc, char const *argv[])
{
	launch_CLI();
	return 0;
}



