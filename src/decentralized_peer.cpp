#include "peer.hpp"
#include "messages.hpp"
#include <mutex>
#include <stdexcept>
#include "constants.hpp"
#include <dirent.h>
#include <string>
#include <thread>
#include <vector>

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

void DecentralizedPeer::broadcast_query(conn_t sender, msg_t* message, msg_t* query_response) {
	std::vector<std::thread> threads;
	create_message(query_response, NULL_MSG);

	// broadcast to everyone except sender
	for(auto neighbor : neighbors) {
		if(neighbor.addr == sender.addr)
			continue;

		threads.emplace_back(std::thread([neighbor, message, &query_response] () {
			msg_t response = send_and_recv(neighbor, *message);

			// we don't lock bc only one response should ever backtrace
			if(response.type != DUP_REQUEST) {
				query_response->buf = response.buf;
				query_response->size = response.size;
				query_response->type = response.type;
			}
		}));
	}

	for(auto &thread : threads)
		thread.join();
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
        broadcast_query(client, &request, &response);

		if(response.type == NULL_MSG)
			create_message(&response, DUP_REQUEST);

		send_msg(response, client);
		delete_msg(&response);
        return;
    }

	create_message(&response, msg_id, server.get_conn_info(), STATUS_OK);
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
	msg_t request, response;
	conn_t peer;

	msg_id = get_message_id();
	create_message(&request, msg_id, filename, SEARCH_INDEX);

	{
		std::unique_lock<std::mutex> lock(query_map_lock);
		received_queries.insert({ msg_id, server.get_conn_info() });
	}

	broadcast_query(server.get_conn_info(), &request, &response);
	delete_msg(&request);

	if(response.type == STATUS_OK) {
		parse_message(&response, &msg_id,  &peer);
		delete_msg(&response);
		return peer;
	}

	delete_msg(&response);
	return { -1, -1, -1 };
}

void DecentralizedPeer::message_handler(conn_t client, msg_t request) {

#ifdef DEBUG
	printf("Received a message of type %d\n", request.type);
#endif

	switch(request.type) {
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