#include "comms.h"
#include "peer.hpp"

// class DecentralizedPeer : public IPeer {
//     private:
//         Server server;
//         FileIndex file_index;
//         std::unordered_map<int, conn_t> received_queries;
//         std::vector<conn_t> neighbors;
//         void broadcast_query(conn_t sender, msg_t message);
//         void backtrace_response(conn_t sender, msg_t message);
//         void message_handler(conn_t, msg_t);
//     public:
//         DecentralizedPeer(std::string neighbor_file);
//         int request_file(std::string);
//         conn_t search_for_file(std::string);
// };

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