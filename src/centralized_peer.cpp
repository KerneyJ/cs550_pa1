#include "peer.hpp"
#include "thread_pool.hpp"
#include <cstring>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string>
#include <cstring>
#include "constants.hpp" 
#include "messages.hpp"

#define MAX_DIR_NAME_SIZE 1024

CentralizedPeer::CentralizedPeer(conn_t index_server) {
	this->index_server = index_server;

   	auto fp = std::bind(&CentralizedPeer::message_handler, this, std::placeholders::_1, std::placeholders::_2);
	server.start(fp, false);
	
	register_user();
}

int CentralizedPeer::register_user() {
	msg_t request;
	int status;

	create_message(&request, server.get_conn_info(), NEW_USER);
	status = send_once(index_server, request);
	delete_msg(&request);

	return status;
}

int CentralizedPeer::register_directory(std::string directory_name) {
	DIR *dir;
	struct dirent *entry;
	int files_registered = 0;
	dir = opendir(directory_name.c_str());

	if (dir == NULL) {
		perror("opendir");
		printf("Please ensure that the shared file directory exists.\n");
		return 1;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_REG)
			continue;

		if (register_file(entry->d_name) < 0) {
			closedir(dir);
			return -1; //error registering file
		}

		files_registered++;
	}

	closedir(dir);
	return files_registered;
}

int CentralizedPeer::register_file(std::string filename) {
	msg_t request;
	int status;

	create_message(&request, filename, server.get_conn_info(), REGISTER_FILE);
	status = send_once(index_server, request);
	delete_msg(&request);

	return status;
}

conn_t CentralizedPeer::search_for_file(std::string filename) {
	conn_t peer;
	msg_t request, response;

	create_message(&request, filename, SEARCH_INDEX);
	response = send_and_recv(index_server, request);
	delete_msg(&request);
	
	if(response.type == STATUS_BAD || response.type == NULL_MSG) {
		delete_msg(&response);
		return { -1, -1, -1 };
	}

	parse_message(&response, &peer);
	delete_msg(&response);

	return peer;
}

int CentralizedPeer::request_file(std::string filename) {
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

int CentralizedPeer::request_file(conn_t peer, std::string filename) {
	msg_t request, response;

	create_message(&request, filename, REQUEST_FILE);
	response = send_and_recv(peer, request);
	delete_msg(&request);

	if(response.type == NULL_MSG) {
		perror("Message failed to receive.\n");
		delete_msg(&response);
		return -1;
	}

	if(response.type == STATUS_BAD) {
		perror("Peer did not have requested file.\n");
		delete_msg(&response);
		return -1;
	}

#ifdef DEBUG
	printf("File successfully downloaded!\n");
#endif

	delete_msg(&response);
	return 0;
}

int CentralizedPeer::send_file(conn_t client, msg_t request) {
	msg_t response, error;
	char path[256] = {0};

	sprintf(path, "%s/", SHARED_FILE_DIR);
	memcpy(path + strlen(SHARED_FILE_DIR) + 1, request.buf, request.size);

	if (createfile_msg(&response, path) < 0) {
		create_message(&error, STATUS_BAD);
		send_msg(error, client);
		delete_msg(&error);

		printf("File could not be sent! Path=%s\n", path);
		return -1;
	}

	send_msg(response, client);
	delete_msg(&response);

#ifdef DEBUG
	printf("File successfully sent!\n");
#endif

	return 0;
}

int CentralizedPeer::replicate_file(conn_t client, msg_t request) {
	conn_t peer;
	std::string filename;
	
	parse_message(&request, &filename, &peer);

#ifdef DEBUG
	printf("Attempting to replicate file {%s}!\n", filename.c_str());
#endif

	if(request_file(peer, filename) < 0) {
		printf("Failed to replicate. :(\n");
		return -1;
	}

	printf("request file worked!\n");

	return register_file(filename);
}

void CentralizedPeer::message_handler(conn_t client, msg_t request) {

#ifdef DEBUG
	printf("Received a message of type %d\n", request.type);
#endif

	switch(request.type) {
		case REQUEST_FILE:
			send_file(client, request);
			break;
		case REPLICATION_REQ:
			replicate_file(client, request);
			break;
		default:
			printf("I don't know what to do with message type %d\n", request.type);
	}
}