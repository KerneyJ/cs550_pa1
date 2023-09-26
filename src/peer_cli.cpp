#include <stdio.h> 
#include "peer.hpp"
#include "constants.hpp"
#include <string.h>

#define MAX_DIR_NAME_SIZE 1024


int launch_CLI() {

	char *ip = INDEX_SERVER_IP;
	int port = INDEX_SERVER_PORT;
	char *file_dir = SHARED_FILE_DIR;
	char search_filename[MAX_DIR_NAME_SIZE];
	int is_running = 1;
	int user_input;
	char local_shared_dir[MAX_DIR_NAME_SIZE];
	char file_to_register[MAX_DIR_NAME_SIZE];
	char full_file_path[2*MAX_DIR_NAME_SIZE];
	char y_or_n = 'n';
	//Welcome
	printf("\n-------------------------------------------------\n");
	printf("Howdy partner 🤠, ready to pirate some 💿💿💿?");
	printf("\n-------------------------------------------------\n");
	while (y_or_n != 'y') {
		printf("\n🚀 To start, enter the name of the file sharing directory you'd like to use: ");
		scanf("%s", &local_shared_dir);
		printf("Ok. Your shared directory is: {%s}. Is that right? [y/n]", local_shared_dir);
		scanf("%c", &y_or_n);
	}
	printf("Alright then, giiiiddy up! 🤠🤠🤠🤠");
	
	while (is_running) {
		//Main menu

		printf("\n\nSelect a menu item:\n");
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
			printf("📝 Enter the name of the file you'd like to register: \n");
			scanf("%s", &file_to_register);
			if (register_file(file_to_register) == 0) {
				printf("🔥 Sent a message to the index server to register the file!");
			}
			else {
				printf("Error registering file...");
			}
		}
		else if (user_input == 3) {
			printf("🔎 Enter file you'd like to search for: ");
			//read in the filename from user
			scanf("%s", &search_filename);
			conn_t reply = search_for_file(search_filename);
			if (reply.addr == -1) {
				printf("file named {%s} doesn't exist.", search_filename);
			}
			else {
				printf("File found on host at IP {%d}, port {%d}", reply.addr, reply.port);
			}
			
		}
		else if (user_input ==4) {
			printf("Requesting a file to download...\n");
			printf("📦 Enter the file you'd like to download: ");
			scanf("%s", &search_filename);
			conn_t reply = search_for_file(search_filename);
			if (reply.addr == -1) {
				printf("file named {%s} doesn't exist.", search_filename);
			}
			else 
				printf("File found on host at IP {%d}, port {%d}... Downloading...", reply.addr, reply.port);
				//TODO: Request file from peer!

			}

		}
	}
	return 0;
}



int main(int argc, char const *argv[])
{
	launch_CLI();
	return 0;
}



