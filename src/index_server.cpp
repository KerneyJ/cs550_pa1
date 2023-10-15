#include <signal.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <mutex>
#include <set>
#include "file_index.hpp"
#include "thread_pool.hpp"
#include "server.hpp"
#include "constants.hpp"
#include "messages.hpp"

extern "C" {
	#include "comms.h"
}

static void register_user(conn_t client, msg_t message);
static void register_file(conn_t client, msg_t message);
static void search_index(conn_t client, msg_t message);
static void request_replication(conn_t peer_with_file, std::string filename, int replications_left);
static msg_t create_replication_msg(std::string filename, conn_t peer);

static Server server;
static FileIndex file_index;

static std::vector<conn_t> peers;
static std::mutex peer_lock;

static void message_handler(conn_t client, msg_t request) {
	unsigned char* ip = (unsigned char*) &client.addr;
#ifdef DEBUG
	printf("Received a message of type %d\n", request.type);
#endif
	switch (request.type) {
		case NEW_USER:
			return register_user(client, request);
		case REGISTER_FILE:
			return register_file(client, request);
		case SEARCH_INDEX:
			return search_index(client, request);
		default:
			fprintf(stderr, "Unknown message type: %d", request.type);
	}
}

int main(int argc, char** argv) {
	server.start(message_handler);

	return 0;
}

void register_user(conn_t client, msg_t request) {
	conn_t peer;

	peer = msg_to_conn(request);

	std::unique_lock<std::mutex> lock(peer_lock);
	peers.push_back(peer);

#ifdef DEBUG
	unsigned char* ip = (unsigned char*) &peer.addr;
	printf("Added peer: %d.%d.%d.%d:%d, Peer count: %lu\n", ip[0], ip[1], ip[2], ip[3], peer.port, peers.size());
#endif
}

void register_file(conn_t client, msg_t request) {

	auto [filename, peer] = msg_to_str_and_conn(request);

	int num_peers = file_index.add_peer(filename, peer);

#ifdef DEBUG
	printf("Registered peer (%d, %d) with file %s. Total peers with file: %d\n", 
		peer.addr, peer.port, filename.data(), file_index.count_peers(filename));
#endif

	if(num_peers < file_index.replication_factor) {
		request_replication(peer, filename, file_index.replication_factor - num_peers);
	}
}

void search_index(conn_t client, msg_t request) {
	msg_t response;
	std::string filename;
	
	filename = msg_to_str(request);

#ifdef DEBUG
	printf("Searching index for file: %s\n", filename.c_str());
#endif

	conn_t peer = file_index.get_rand_peer(filename);

	if(peer.addr == -1) {
		create_message(&response, NULL, STATUS_BAD);
	} else {
		response = conn_to_msg(peer, STATUS_OK);
	}
	
	send_msg(response, client);
	delete_msg(&response);
}

std::vector<conn_t> find_replication_peers(std::string filename, int num_peers) {
	std::unique_lock<std::mutex> lock(peer_lock);

	if(peers.empty())
		return {};

	std::vector<conn_t> valid_peers;
	conn_t valid_peer;
	int peer_idx, start_idx, is_valid = 0;
	peer_idx = start_idx = rand() % peers.size();

	// attempt to find enough peers without this file to replicate to
	for (int i = 0; i < num_peers; i++) {
		do {
			valid_peer = peers.at(peer_idx);
			is_valid = !file_index.contains_peer(filename, valid_peer);

			peer_idx = (peer_idx + 1) % peers.size();
			// we've tried every peer, just use the ones that we've found
			if(peer_idx == start_idx) {
				i = num_peers;
				break;
			}
		} while(!is_valid);

		if(is_valid)
			valid_peers.push_back(valid_peer);
	}

	return valid_peers;
}

void request_replication(conn_t peer_with_file, std::string filename, int replications_left) {
	conn_t client;
	msg_t message;
	auto valid_peers = find_replication_peers(filename, replications_left);

#ifdef DEBUG
	if(valid_peers.size() < replications_left) {
		printf("Found %lu/%d peers to replicate file %s to.\n", valid_peers.size(), replications_left, filename.c_str());
	}
#endif

	message = create_replication_msg(filename, peer_with_file);
	for(auto peer : valid_peers) {
#ifdef DEBUG
		printf("Sending replication message to peer (%d, %d)\n", peer.addr, peer.port);
#endif
		clntinitco_conn(&client, &peer);
		send_msg(message, client);
		close_conn(&client);
	}
	delete_msg(&message);
}