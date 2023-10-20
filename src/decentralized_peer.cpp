#include <exception>
#include <vector>
#include <mutex>
#include <stdexcept>
#include <dirent.h>
#include <string>
#include <thread>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "peer.hpp"
#include "messages.hpp"
#include "constants.hpp"

DecentralizedPeer::DecentralizedPeer(unsigned char peer_id, std::string adjacency_config) {
	this->peer_id = peer_id;

	this->init_neighbors(adjacency_config);
	this->init_fileset();

	auto fp = std::bind(&DecentralizedPeer::message_handler, this, std::placeholders::_1, std::placeholders::_2);
	
	if(server.start(fp, false) < 0)
		throw std::logic_error("Server could not bind.");
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
	create_message(query_response, DUP_REQUEST);

	// broadcast to everyone except sender
	for(auto neighbor : neighbors) {
		if(neighbor.addr == sender.addr)
			continue;

		threads.emplace_back(std::thread([neighbor, message, &query_response] () {
			unsigned char* ip = (unsigned char*) &neighbor.addr;
#ifdef DEBUG
			printf("Sending message to neighbor: %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], neighbor.port);
#endif
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

void DecentralizedPeer::send_file(conn_t client, msg_t request) {
	msg_t response, error;
	char path[256] = {0};

	sprintf(path, "%s/", SHARED_FILE_DIR);
	memcpy(path + strlen(SHARED_FILE_DIR) + 1, request.buf, request.size);

	if (createfile_msg(&response, path) < 0) {
		create_message(&error, STATUS_BAD);
		send_msg(error, client);
		delete_msg(&error);

		printf("File could not be sent! Path=%s\n", path);
		return;
	}

	send_msg(response, client);
	delete_msg(&response);

#ifdef DEBUG
	printf("File successfully sent!\n");
#endif

	return;
}

int DecentralizedPeer::request_file(std::string filename) {
	conn_t peer;
	msg_t request, response;

	peer = search_for_file(filename);

	if(peer.addr == -1) {
		printf("No peers with requested file.\n");
		return -1;
	}

	create_message(&request, filename, REQUEST_FILE);

	response = send_and_recv(peer, request);
	delete_msg(&request);

	if(response.type == NULL_MSG) {
		delete_msg(&response);
		printf("Message failed to receive.\n");
		return -1;
	}

	if(response.type == STATUS_BAD) {
		delete_msg(&response);
		printf("Peer did not have requested file.\n");
		return -1;
	}

#ifdef DEBUG
	printf("File successfully downloaded!\n");
#endif

	delete_msg(&response);
	return 0;
}

void DecentralizedPeer::search_index(conn_t client, msg_t request) {
	msg_t response;
	int msg_id;
	std::string filename;

	parse_message(&request, &msg_id, &filename);
#ifdef DEBUG
	printf("Received request for file: %s. (msg %d)\n", filename.c_str(), msg_id);
#endif
	{
		// Check if we have already seen this message
		std::unique_lock<std::mutex> lock(query_map_lock);
		if(received_queries.find(msg_id) != received_queries.end()) {
#ifdef DEBUG
			printf("Duplicate request (msg %d)\n", msg_id);
#endif
			create_message(&response, DUP_REQUEST);
			send_msg(response, client);
			delete_msg(&response);
			return;
		}

		received_queries.insert({ msg_id, { client.addr, FIXED_PORT } });
	}

	// Check if we have the file. if not, broadcast this message
    if(file_set.find(filename) == file_set.end()) {
#ifdef DEBUG
		printf("File not here, broadcasting query to the homies...\n");
#endif
        broadcast_query(client, &request, &response);

#ifdef DEBUG
		if(response.type == NULL_MSG) {
			printf("Homies didn't have it...\n");
		}
		printf("Forward the response\n");
#endif
		send_msg(response, client);
		delete_msg(&response);
        return;
    }
#ifdef DEBUG
	printf("We have the file!\n");
#endif

	create_message(&response, msg_id, server.get_conn_info(), STATUS_OK);
    send_msg(response, client);
	delete_msg(&response);
}

conn_t DecentralizedPeer::search_for_file(std::string filename) {
	int msg_id;
	msg_t request, response;
	conn_t peer;
#ifdef DEBUG
	printf("Searching for file %s...\n", filename.c_str());
#endif
	// The file is on this peer
	if(file_set.find(filename) != file_set.end())
		return server.get_conn_info();
#ifdef DEBUG
	printf("File not found locally...\n");
#endif
	msg_id = get_message_id();
	create_message(&request, msg_id, filename, SEARCH_INDEX);

	{
		std::unique_lock<std::mutex> lock(query_map_lock);
		received_queries.insert({ msg_id, server.get_conn_info() });
	}
#ifdef DEBUG
	printf("Broadcasting query...\n");
#endif
	broadcast_query(server.get_conn_info(), &request, &response);
	delete_msg(&request);

	if(response.type == STATUS_OK) {
#ifdef DEBUG
		printf("Got a valid response from the broadcast!\n");
#endif
		parse_message(&response, &msg_id,  &peer);
		delete_msg(&response);
		return peer;
	}
#ifdef DEBUG
	printf("Nobody had the file. :(\n");
#endif
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
