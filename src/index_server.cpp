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

extern "C" {
	#include "comms.h"
}

#define REPLICATION_FACTOR 2

static void register_user(conn_t client, msg_t message);
static void register_file(conn_t client, msg_t message);
static void search_index(conn_t client, msg_t message);
static void request_replication(std::string filename);
static msg_t create_replication_msg(std::string filename, conn_t peer);

static volatile sig_atomic_t stop;
static conn_t server_conn;

static FileIndex file_index;

static std::vector<conn_t> peers;
static std::mutex peer_lock;

static std::set<std::string> replication_waitlist;
static std::mutex waitlist_lock;

static void set_stop_flag(int signum) {
	printf("Catching SIGINT!\n");
	close_conn(&server_conn); // exits blocking accept() calls
    stop = 1;
}

static void message_handler(conn_t client_conn, msg_t msg) {
	unsigned char* ip = (unsigned char*) &client_conn.addr;
#ifdef DEBUG
	printf("Received a message of type %d\n", msg.type);
#endif
	switch (msg.type) {
		case NEW_USER:
			return register_user(client_conn, msg);
		case REGISTER_FILE:
			return register_file(client_conn, msg);
		case SEARCH_INDEX:
			return search_index(client_conn, msg);
		default:
			fprintf(stderr, "Unknown message type: %d", msg.type);
	}
}

int main(int argc, char** argv) {

    signal(SIGINT, set_stop_flag);

    char *ip = INDEX_SERVER_IP;
    int port = INDEX_SERVER_PORT;

    if(servinit_conn(&server_conn, ip, port) < 0) {
		printf("Failed to initialize server, shutting down.\n");
		return -1;
	}

	if(servlstn_conn(&server_conn, 5)) {
		printf("Failed to start listening, shutting down.\n");
		return -1;
	}
	
	servloop_conn(&server_conn, &message_handler, &stop);

	printf("Shutting down!\n");
	close_conn(&server_conn);

	return 0;
}

static std::string buf_to_string(char* buffer, size_t len) {
	char test[256] = {0};
	memcpy(test, buffer, len);

	return std::string(test);
}

void register_user(conn_t client, msg_t message) {
	std::unique_lock<std::mutex> lock(peer_lock);
	peers.push_back(client);
#ifdef DEBUG
	printf("Added peer, num_peers = %lu\n", peers.size());
#endif
}

void register_file(conn_t client, msg_t message) {
	conn_t peer_server;

	int* ibuffer = (int*) message.buf;
	if(ibuffer[0] >= 0) {
		peer_server.addr = ibuffer[0];
		peer_server.port = ibuffer[1];
	} else {
		peer_server = client;
	}

	std::string filename = buf_to_string(message.buf + sizeof(int)*2, message.size - sizeof(int)*2);

	file_index.add_peer(filename, peer_server);
#ifdef DEBUG
	printf("Registered peer (%d, %d) with file %s. Total peers with file: %d\n", 
#endif
		peer_server.addr, peer_server.port, filename.data(), file_index.count_peers(filename));

	// if(file_index.count_peers(filename) < REPLICATION_FACTOR) {
	// 	request_replication(filename);

	// 	std::unique_lock<std::mutex> lock(waitlist_lock);
	// 	{
	// 		replication_waitlist.insert(filename);
	// 	}
	// } else {
	// 	std::unique_lock<std::mutex> lock(waitlist_lock);
	// 	{
	// 		if(replication_waitlist.find(filename) != replication_waitlist.end())
	// 			replication_waitlist.erase(filename);
	// 	}
	// }
}

void search_index(conn_t client, msg_t message) {
	msg_t res;
	std::string filename = buf_to_string(message.buf, message.size);
#ifdef DEBUG
	printf("searching for file %s. filename length=%lu\n", filename.data(), message.size);
#endif
	conn_t peer = file_index.get_rand_peer(filename);

	if(peer.addr == -1) {
		create_message(&res, "", STATUS_BAD);
	} else {
		int peer_data[2] = {peer.addr, peer.port};
		createupdt_msg(&res, (char*) peer_data, sizeof(int) * 2, STATUS_OK);
	}
#ifdef DEBUG
	printf("returning message: type=%d, size=%lu\n", res.type, res.size);
#endif
	send_msg(res, client);
	delete_msg(&res);
}

// std::vector<conn_t> find_replication_peers(std::string filename) {
// 	std::vector<conn_t> valid_peers;
// 	conn_t valid_peer;
// 	uint peer_idx, start_idx;
// 	peer_idx = start_idx = rand() % peers.size();

// 	std::unique_lock<std::mutex> lock(peer_lock);
// 	// attempt to find enough peers without this file to replicate to
// 	for (uint i = 0; i < REPLICATION_FACTOR - file_index->count_peers(filename); i++) {
// 		do {
// 			valid_peer = peers.at(peer_idx);
// 			peer_idx++;

// 			// we've tried every peer, just use the ones that we've found
// 			if(peer_idx == start_idx) {
// 				valid_peer = {-1, -1, -1};
// 				break;
// 			}
// 		} while(file_index->contains_peer(filename, valid_peer));

// 		if(valid_peer.addr < 0)
// 			break;

// 		valid_peers.push_back(valid_peer);
// 	}

// 	return valid_peers;
// }

// msg_t create_replication_msg(std::string filename, conn_t peer) {
// 	size_t msg_size = sizeof(int) * 2 + sizeof(filename);
// 	char* msg_buffer = (char*) malloc(msg_size);
// 	msg_buffer[0] = peer.addr;
// 	msg_buffer[sizeof(int)] = peer.port;
// 	memcpy(msg_buffer + sizeof(int) * 2, filename.data(), sizeof(filename));

// 	return { msg_buffer, msg_size, REPLICATION_REQ };
// }

// void request_replication(std::string filename) {
// 	msg_t message;
// 	auto valid_peers = find_replication_peers(filename);

// 	for(auto peer : valid_peers) {
// 		auto peer_with_file = file_index->get_rand_peer(filename);

// 		message = create_replication_msg(filename, peer);

// 		send_msg(message, peer_with_file);

// 		delete_msg(&message);
// 	}
// }