#include "peer.hpp"
#include "messages.hpp"
#include <mutex>
#include <stdexcept>
#include "constants.hpp"
#include <dirent.h>

DecentralizedPeer::DecentralizedPeer(unsigned char peer_id) {
	this->peer_id = peer_id;

	init_neighbors();
	init_fileset();

   	auto fp = std::bind(&DecentralizedPeer::message_handler, this, std::placeholders::_1, std::placeholders::_2);
	server.start(fp, false);
}

void DecentralizedPeer::init_neighbors() {
	// TODO: use peer id to initialize neighbor list
	// ADJACENCY_CONFIG_PATH & PEER_IP_CONFIG_PATH defined in constants.hpp
}

void DecentralizedPeer::init_fileset() {
	DIR *dir;
	struct dirent *entry;
	dir = opendir(SHARED_FILE_DIR);

	if (dir == NULL) {
		perror("opendir");
		printf("Please ensure that the shared file directory exists.\n");
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_REG)
			continue;

		file_set.insert(entry->d_name);
	}
}

int DecentralizedPeer::get_message_id() {
	std::unique_lock<std::mutex> lock(message_id_lock);
	message_count++;
	return message_count + ((unsigned int) peer_id << 24);
}

void DecentralizedPeer::broadcast_query(conn_t sender, msg_t message) {
	// broadcast to everyone except sender
	for(auto neighbor : neighbors) {
		if(neighbor.addr == sender.addr)
			continue;

		send_once(neighbor, message);
	}
}

void DecentralizedPeer::backtrace_response(conn_t sender, msg_t message) {
	int msg_id;
	conn_t peer;

	std::unique_lock<std::mutex> lock(query_map_lock);
	parse_message(&message, &msg_id);

	auto iter = received_queries.find(msg_id);

	if(iter == received_queries.end()) {
		printf("Backtrace message has no corresponding peer.\n");
		return;
	}

	if(iter->second.addr == server.get_conn_info().addr) {
		printf("Backtrace message arrived at its destination!\n");
		// TODO: respond in cli
	}

	send_once(iter->second, message);
}

void DecentralizedPeer::search_index(conn_t client, msg_t request) {
	msg_t response;
	int msg_id;
    std::string filename;

    parse_message(&request, &msg_id, &filename);

	{
		// Check if we have already seen this message
		std::unique_lock<std::mutex> lock(query_map_lock);
		if(received_queries.find(msg_id) != received_queries.end())
			return;

		received_queries.insert({ msg_id, { client.addr, FIXED_PORT } });
	}

	// Check if we have the file. if not, broadcast this message
    if(file_set.find(filename) == file_set.end()) {
        broadcast_query(client, request);
        return;
    }

	create_message(&response, get_message_id(), server.get_conn_info(), STATUS_OK);
    send_msg(response, client);
	delete_msg(&response);
}

void DecentralizedPeer::send_file(conn_t client, msg_t request) {
	
}

int DecentralizedPeer::request_file(std::string filename) {
    return -1;
}

conn_t DecentralizedPeer::search_for_file(std::string filename) {
	int msg_id;
	msg_t request;

	msg_id = get_message_id();
	create_message(&request, msg_id, filename, SEARCH_INDEX);

	{
		std::unique_lock<std::mutex> lock(query_map_lock);
		received_queries.insert({ msg_id, server.get_conn_info() });
	}

	broadcast_query(server.get_conn_info(), request);
	delete_msg(&request);

    return {-1, -1, -1};
}

void DecentralizedPeer::message_handler(conn_t client, msg_t request) {

#ifdef DEBUG
	printf("Received a message of type %d\n", request.type);
#endif

	switch(request.type) {
		case STATUS_OK:
		case STATUS_BAD:
			backtrace_response(client, request);
			break;
		case SEARCH_INDEX:
			search_index(client, request);
			break;
		case REQUEST_FILE:
			send_file(client, request);
			break;
		default:
			printf("I don't know what to do with message type %d\n", request.type);
	}
}

/*** Implementation necessary for interface. ***/

int DecentralizedPeer::register_user() {
    throw std::logic_error("Function not implemented for decentralized peer.");
}

int DecentralizedPeer::register_directory(std::string) {
    throw std::logic_error("Function not implemented for decentralized peer.");
}

int DecentralizedPeer::register_file(std::string) {
    throw std::logic_error("Function not implemented for decentralized peer.");
}