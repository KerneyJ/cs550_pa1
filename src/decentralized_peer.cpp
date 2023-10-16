#include "peer.hpp"
#include "messages.hpp"
#include <stdexcept>

DecentralizedPeer::DecentralizedPeer(unsigned char peer_id) {
	id = peer_id;

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
	// TODO: populate file_set with files in ./data directory
}

void DecentralizedPeer::broadcast_query(conn_t sender, msg_t message) {
	// TODO: broadcast to everyone except sender
}

void DecentralizedPeer::backtrace_response(conn_t sender, msg_t message) {
	// TODO: send msg back to its origin
}

void DecentralizedPeer::search_index(conn_t client, msg_t request) {
	msg_t response;
	int msg_id;
    std::string filename;

    parse_message(&request, &msg_id, &filename);

	// TODO: make this threadsafe
	if(received_queries.find(msg_id) != received_queries.end())
		return;

    if(file_set.find(filename) == file_set.end()) {
        broadcast_query(client, request);
        return;
    }

	// response = create_message(id, server.get_conn_info(), STATUS_OK);

    // send_msg(response, client);
}

void DecentralizedPeer::send_file(conn_t client, msg_t request) {
	
}

int DecentralizedPeer::request_file(std::string filename) {
    return -1;
}

conn_t DecentralizedPeer::search_for_file(std::string filename) {
    return {-1, -1, -1};
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