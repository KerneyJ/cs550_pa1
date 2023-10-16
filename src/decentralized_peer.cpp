#include "peer.hpp"
#include <stdexcept>

DecentralizedPeer::DecentralizedPeer(std::string) {
}

void DecentralizedPeer::broadcast_query(conn_t sender, msg_t message) {
}

void DecentralizedPeer::backtrace_response(conn_t sender, msg_t message) {
}

int DecentralizedPeer::request_file(std::string) {
    return -1;
}

conn_t DecentralizedPeer::search_for_file(std::string) {
    return {-1, -1, -1};
}


int DecentralizedPeer::register_user() {
    throw std::logic_error("Function not implemented for decentralized peer.");
}

int DecentralizedPeer::register_directory(std::string) {
    throw std::logic_error("Function not implemented for decentralized peer.");
}

int DecentralizedPeer::register_file(std::string) {
    throw std::logic_error("Function not implemented for decentralized peer.");
}