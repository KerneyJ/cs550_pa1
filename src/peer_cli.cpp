#include <arpa/inet.h>
#include <stdio.h>
#include "peer.hpp"
#include "constants.hpp"
#include "server.hpp"
#include "benchmarks.hpp"

#define MAX_DIR_NAME_SIZE 1024


int launch_CLI(conn_t peer_server) {

	char search_filename[MAX_DIR_NAME_SIZE];
	int is_running = 1;
	int user_input;
	char file_to_register[MAX_DIR_NAME_SIZE];
	
	//Welcome
	printf("\n-------------------------------------------------\n");
	printf("Howdy partner 🤠, ready to pirate some 💿💿💿?");
	printf("\n-------------------------------------------------\n");
	
	printf("Your shared file directory location is %s", SHARED_FILE_DIR);

	while (is_running) {
		//Main menu

		printf("\n\n\nSelect a menu item:\n");
		printf("--------------------\n");
		printf("[1] ✌️ Spread the love. Register your shared directory with the server.\n");
		printf("[2] 💰 Register a file.\n");
		printf("[3] 🔎 Search for a file on the server index.\n");
		printf("[4] 📦 Request a file to download from the network.\n");
		printf("[5] 🧪 Run a benchmark.\n");
		printf("[9] 👋 Quit!\n");

		scanf("%d", &user_input);
		if (user_input == 9) {
			printf("Aye Matey, Farewell! 👋\n\n");
			is_running = 0;
		} else if (user_input == 1) {
			printf("\nRegistering files with the server...");
			int files_registered = register_dir(SHARED_FILE_DIR);
			if (files_registered <0) {
				printf("Error registering files.");
			}
			else {
				printf("✅  %d filenames sent to server for registration", files_registered);
			}
		} else if (user_input == 2) {
			printf("📝 Enter the name of the file you'd like to register: \n");
			scanf("%s", &file_to_register);
			if (register_file(file_to_register) == 0) {
				printf("🔥 Sent a message to the index server to register the file!");
			}
			else {
				printf("Error registering file...");
			}
		} else if (user_input == 3) {
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
		} else if (user_input == 4) {
			printf("Requesting a file to download...\n");
			printf("📦 Enter the file you'd like to download: ");
			scanf("%s", &search_filename);
			conn_t reply = search_for_file(search_filename);
			if (reply.addr == -1) {
				printf("file named {%s} doesn't exist.", search_filename);
			}
			else {
				printf("File found on host at IP {%d}, port {%d}... Downloading...", reply.addr, reply.port);

				if(request_file_from_peer(reply, search_filename) < 0) {
					printf("Failed to download.\n");
					continue;
				}
				printf("Downloaded file: %s", search_filename);
				register_file(search_filename);
			}
		} else if (user_input == 5) {
			int test_id, vm_id, vm_target, num_files;
			char file_size;
			std::time_t start;

			printf("\nSelect a benchmark to run:\n");
			printf("--------------------\n");
			printf("[1] Search files.\n");
			printf("[2] Search and download files.\n");
			scanf("%d", &test_id);

			printf("\nEnter the VM id for this peer, one digit (0-9):\n");
			scanf("%d", &vm_id);

			printf("\nEnter the VM id for to target for searches & downloads, one digit (0-9):\n");
			scanf("%d", &vm_target);

			printf("\nEnter the number of files for this test (1-1,000,000):\n");
			scanf("%d", &num_files);

			printf("\nEnter the size of the files for this test, 1 character (K, M, or G):\n");
			scanf("%s", &file_size);

			printf("\nEnter the UNIX timestamp of when to run:\n");
			scanf("%lu", &start);

			run_benchmark(test_id, vm_id, vm_target, num_files, file_size, start);
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	conn_t peer_server;
	conn_t index_server;
	int test_no;

	if (parse_conn_arg(argc, argv, 1, &index_server) < 0 ||
		parse_conn_arg(argc, argv, 2, &peer_server) < 0) {
		printf("Please provide the ip and port that the index server is running on followed by the ip and port that this peer's server is running.\n");
		printf("\teg: ./peer_cli 185.236.36.234:8080 127.0.0.1:8080\n");
		printf("\t       	    index server ⤴     peer server ⤴   \n");
		return -1;
	}

	set_index_server_conn(index_server);
	set_peer_server_conn(peer_server);

	launch_CLI(peer_server);
	return 0;
}



