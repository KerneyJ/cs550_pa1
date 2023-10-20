#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdio.h>
#include "constants.hpp"
#include "server.hpp"
#include "benchmarks.hpp"
#include "peer.hpp"

IPeer* peer;

void (*menu_items[10]) () = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
const char* menu_labels[10]  = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };


static void register_directory() {
	printf("\nRegistering files with the server...\n");

	int files_registered = peer->register_directory(SHARED_FILE_DIR);

	if (files_registered < 0) {
		printf("Error registering files.\n");
		return;
	}
		
	printf("✅  %d filenames sent to server for registration\n", files_registered);
}

static void register_file() {
	std::string filename;

	printf("📝 Enter the name of the file you'd like to register: \n");
	getline(std::cin, filename);

	if (peer->register_file(filename) < 0) {
		printf("Error registering file...\n");
		return;
	}
		
	printf("🔥 Sent a message to the index server to register the file!\n");
}

static void search_for_file() {
	std::string filename;
	conn_t result;

	printf("🔎 Enter file you'd like to search for: \n");
	getline(std::cin, filename);

	result = peer->search_for_file(filename);
	if (result.addr == -1) {
		printf("File named {%s} doesn't exist.\n", filename.c_str());
		return;
	}

	unsigned char* ip = (unsigned char*) &result.addr;
	printf("File found on host at: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], result.port);
}

static void request_file() {
	std::string filename;
	conn_t result;

	printf("📦 Enter the file you'd like to download: \n");
	getline(std::cin, filename);

	if (peer->request_file(filename) < 0) {
		printf("Error retrieving file. :(\n");
		return;
	}
		
	printf("Downloaded file: %s\n", filename.c_str());
	peer->register_file(filename);
}

void launch_cli() {
	std::string input;
	int menu_choice;

	//Welcome
	printf("\n-------------------------------------------------\n");
	printf("Howdy partner 🤠, ready to pirate some 💿💿💿?");
	printf("\n-------------------------------------------------\n");
	
	printf("Your shared file directory location is %s", SHARED_FILE_DIR);

	while (true) {
		//Main menu

		printf("\n\n\nSelect a menu item:\n");
		printf("--------------------\n");
		for(int i = 0; i < 10; i++) {
			if(menu_labels[i] != NULL)
				printf("[%d] %s\n", i, menu_labels[i]);
		}

		getline(std::cin, input);

		try {
			menu_choice = stoi(input);
		} catch(std::exception e) {
			printf("That is not a valid option, try again.\n");
			continue;
		}

		if(menu_choice == 9) {
			printf("Aye Matey, Farewell! 👋\n\n");
			return;
		} 

		if(menu_choice < 0 || menu_choice > 9 || menu_items[menu_choice] == NULL) {
			printf("That is not a valid option, try again.\n");
			continue;
		}

		menu_items[menu_choice]();
	}
}

int parse_conn_arg(int argc, char** argv, int idx, conn_t* conn) {
	std::string ip, port;
	int split_idx;

	if(argc < idx + 1)
		return -1;

	std::string full_route(argv[idx]);
	split_idx = full_route.find(':');

	if(split_idx == std::string::npos)
		return -1;

	ip = full_route.substr(0, split_idx);
	port = full_route.substr(split_idx + 1, full_route.size() - split_idx);

	conn->addr = inet_addr(ip.data());
	conn->port = stoi(port);

	return 0;
}

int main(int argc, char** argv) {

	#ifdef CENTRALIZED_PEER
		conn_t index_server;

		if (argc < 3 || parse_conn_arg(argc, argv, 2, &index_server) < 0) {
			printf("Please provide a vm id and the ip and port that the index server is running on.\n");
			printf("\teg: ./bin/c_peer <VM_ID> <IDX_SERVER>\n");
			printf("\teg: ./bin/c_peer 1 185.236.36.234:8080\n");
			return -1;
		}

		peer = new CentralizedPeer(atoi(argv[1]), index_server);

		menu_items[1] = register_directory;
		menu_items[2] = register_file;
		menu_items[3] = search_for_file;
		menu_items[4] = request_file;

		menu_labels[1] = "✌️ Spread the love. Register your shared directory with the server.";
		menu_labels[2] = "💰 Register a file.";
		menu_labels[3] = "🔎 Search for a file on the server index.";
		menu_labels[4] = "📦 Request a file to download from the network.";
		menu_labels[9] = "👋 Quit!";
	#elif DECENTRALIZED_PEER

		if(argc < 3)  {
			printf("Please provide a vm id and topology configuration file.\n");
			printf("\teg: ./bin/d_peer <VM_ID> <TOPOLOGY_CFG>\n");
			return -1;
		}

		peer = new DecentralizedPeer(atoi(argv[1]), argv[2]);

		menu_items[1] = search_for_file;
		menu_items[2] = request_file;

		menu_labels[1] = "✌️ 🔎 Search for a file on the server index.";
		menu_labels[2] = "✌️ 📦 Request a file to download from the network.";
		menu_labels[9] = "👋 Quit!";
	#endif

	// no more arguments, launch in cli mode
	if(argc < 4) {
		launch_cli();
		return 0;
	}

	#ifdef CENTRALIZED_PEER
		peer->register_directory(SHARED_FILE_DIR);
	#endif
	
	// parse benchmark id	
	int benchmark_id = atoi(argv[3]);
	run_benchmark(peer, benchmark_id);

	return 0;
}
