#include "peer.hpp"
#include "comms.h"
#include "thread_pool.hpp"
#include <cstring>
#include <functional>
#include <stdio.h>
#include "comms.h"
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

	request = conn_to_msg(server.get_conn_info(), NEW_USER);
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

	request = str_and_conn_to_msg(filename, server.get_conn_info(), REGISTER_FILE);
	auto [t1, t2] = msg_to_str_and_conn(request);
	printf("deserialized: {%d, %d}, %s\n", t2.addr, t2.port, t1.c_str());

	status = send_once(index_server, request);
	delete_msg(&request);

	return status;
}

conn_t CentralizedPeer::search_for_file(std::string filename) {
	conn_t peer;
	msg_t request, response;

	request = str_to_msg(filename, server.get_conn_info(), SEARCH_INDEX);
	response = send_and_recv(index_server, request);
	delete_msg(&request);
	
	if(response.type == STATUS_BAD || response.type == NULL_MSG) {
		return { -1, -1, -1 };
	}

	peer = msg_to_conn(response);

	delete_msg(&response);
	return peer;
}

int CentralizedPeer::request_file(std::string filename) {
	conn_t peer;
	msg_t request, response;
	char cfilename[256] = {0};

	peer = search_for_file(filename);

	if(peer.addr == -1) {
		printf("No peers with requested file.\n");
		return -1;
	}

	strcpy(cfilename, filename.c_str());
	create_message(&request, cfilename, REQUEST_FILE);

	send_and_recv(peer, request);
	delete_msg(&request);

	if(response.type == NULL_MSG) {
		printf("Message failed to receive.\n");
		return -1;
	}

	if(response.type == STATUS_BAD) {
		printf("Peer did not have requested file.\n");
		return -1;
	}

#ifdef DEBUG
	printf("File successfully downloaded!\n");
#endif

	return 0;
}

int CentralizedPeer::request_file(conn_t peer, std::string filename) {
	msg_t request, response;
	char* cfilename;

	strcpy(cfilename, filename.c_str());
	create_message(&request, cfilename, REQUEST_FILE);

	send_and_recv(peer, request);
	delete_msg(&request);

	if(response.type == NULL_MSG) {
		perror("Message failed to receive.\n");
		return -1;
	}

	if(response.type == STATUS_BAD) {
		perror("Peer did not have requested file.\n");
		return -1;
	}

#ifdef DEBUG
	printf("File successfully downloaded!\n");
#endif

	return 0;
}

int CentralizedPeer::send_file(conn_t client, msg_t request) {
	msg_t response, error;
	char path[256] = {0};

	sprintf(path, "%s/", SHARED_FILE_DIR);
	memcpy(path + strlen(SHARED_FILE_DIR) + 1, request.buf, request.size);

	if (createfile_msg(&response, path) < 0) {
		create_message(&error, NULL, STATUS_BAD);
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

	auto [filename, peer] = msg_to_str_and_conn(request);

#ifdef DEBUG
	printf("Attempting to replicate file {%s}!\n", filename);
#endif

	if(request_file(peer, filename) < 0) {
		printf("Failed to replicate. :(\n");
		return -1;
	}

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