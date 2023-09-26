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
	char file_to_register[MAX_DIR_NAME_SIZE];
	

	//Welcome
	printf("\n-------------------------------------------------\n");
	printf("Howdy partner 🤠, ready to pirate some 💿💿💿?");
	printf("\n-------------------------------------------------\n");
	
	printf("Make sure you're running peer_server in a separate shell 🐚.");
	printf("Your shared file directory location is %s", SHARED_FILE_DIR);

	while (is_running) {
		//Main menu

		printf("\n\n\nSelect a menu item:\n");
		printf("--------------------\n");
		printf("[1] ✌️ Spread the love. Register your shared directory with the server.\n");
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
			printf("\nRegistering files with the server...");
			int files_registered = register_dir(SHARED_FILE_DIR);
			if (files_registered <0) {
				printf("Error registering files.");
			}
			else {
				printf("✅  %d filenames sent to server for registration", files_registered);
			}
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
			// printf("Requesting a file to download...\n");
			// printf("📦 Enter the file you'd like to download: ");
			// scanf("%s", &search_filename);
			// conn_t reply = search_for_file(search_filename);
			// if (reply.addr == -1) {
			// 	printf("file named {%s} doesn't exist.", search_filename);
			// }
			// else {
			// 	printf("File found on host at IP {%d}, port {%d}... Downloading...", reply.addr, reply.port);
			// 	//TODO: Request file from peer!

			// }
			conn_t reply = {16777343, 8081, 0};//search_for_file(search_filename);
			char filename[] = "nice_file_to_have";
			request_file_from_peer(reply, filename);
		}
	}
	return 0;
}



int main(int argc, char const *argv[])
{
	launch_CLI();
	return 0;
}



