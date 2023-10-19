#include <vector>
#include <mutex>
#include <stdexcept>
#include <dirent.h>
#include <string>
#include <thread>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "comms.h"
#include "peer.hpp"
#include "messages.hpp"
#include "constants.hpp"

DecentralizedPeer::DecentralizedPeer(unsigned char peer_id, std::string adjacency_config) {
	this->peer_id = peer_id;

	this->init_neighbors(adjacency_config);
	this->init_fileset();

	for(auto neighbor : neighbors) {
		unsigned char* ip = (unsigned char*) &neighbor.addr;
		printf("neighbor at: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], neighbor.port);
	}

	auto fp = std::bind(&DecentralizedPeer::message_handler, this, std::placeholders::_1, std::placeholders::_2);
	server.start(fp, false);
}

unsigned char DecentralizedPeer::get_id() {
	return peer_id;
}

void DecentralizedPeer::init_neighbors(std::string adjacency_config) {
	/* 
	* Adjacency matrix Structure:
	* id: neighbors
	*/
	std::string line;
	std::vector<int> nebvec;
	std::ifstream adjfile(adjacency_config);
	std::ifstream pipfile(PEER_IP_CONFIG_PATH);

	// Parse adjacency file
	while(std::getline(adjfile, line)){
		if(std::stoi(line.substr(0, line.find(','))) == this->peer_id){
			std::string nebstr = line.substr(line.find(":")+1), temp;
			std::stringstream s(nebstr);
			while(std::getline(s, temp, ',')) {
				nebvec.push_back(std::stoi(temp));
			}
		}
	}

	// Parse peer ip config
	while(std::getline(pipfile, line)){
		if(line.find(",") == std::string::npos)
			continue;

		std::string id = line.substr(0, line.find(","));
		if(std::find(nebvec.begin(), nebvec.end(), std::stoi(id)) == nebvec.end())
			continue;

		conn_t conn;
		std::string ip = line.substr(line.find(",")+1);

		conn.addr = inet_addr(ip.data());
		conn.port = FIXED_PORT;

		this->neighbors.push_back(conn);
	}

	if(neighbors.size() == 0) 
		printf("Warning! Could not find any neighbors\n");
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
			unsigned char* ip = (unsigned char*) &neighbor.addr;
			printf("Sending message to neighbor: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], neighbor.port);

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

	printf("Received request for file: %s. (msg %d)\n", filename.c_str(), msg_id);

	{
		// Check if we have already seen this message
		std::unique_lock<std::mutex> lock(query_map_lock);
		if(received_queries.find(msg_id) != received_queries.end()) {
			printf("Duplicate request (msg %d)\n", msg_id);
			create_message(&response, DUP_REQUEST);
			send_msg(response, client);
			delete_msg(&response);
			return;
		}

		received_queries.insert({ msg_id, { client.addr, FIXED_PORT } });
	}

	// Check if we have the file. if not, broadcast this message
    if(file_set.find(filename) == file_set.end()) {
		printf("File not here, broadcasting query to the homies...\n");
        broadcast_query(client, &request, &response);

		if(response.type == NULL_MSG) {
			printf("Homies didn't have it...\n");
			create_message(&response, DUP_REQUEST);
		}

		printf("Forward the response\n");
		send_msg(response, client);
		delete_msg(&response);
        return;
    }

	printf("We have the file!\n");

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

	printf("Searching for file...\n");

	// The file is on this peer
	if(file_set.find(filename) != file_set.end())
		return server.get_conn_info();

	printf("File not found locally...\n");

	msg_id = get_message_id();
	create_message(&request, msg_id, filename, SEARCH_INDEX);

	{
		std::unique_lock<std::mutex> lock(query_map_lock);
		received_queries.insert({ msg_id, server.get_conn_info() });
	}

	printf("Broadcasting query...\n");

	broadcast_query(server.get_conn_info(), &request, &response);
	delete_msg(&request);

	if(response.type == STATUS_OK) {
		printf("Got a valid response from the broadcast!\n");
		parse_message(&response, &msg_id,  &peer);
		delete_msg(&response);
		return peer;
	}

	printf("Nobody had the file. :(\n");

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
