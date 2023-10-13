#include <array>
#include <unordered_map>

extern "C" {
    #include "comms.h"
}

#include "server.hpp"
#include "file_index.hpp"

class PeerBase {
    private:
        conn_t self;
    public:
        PeerBase();
};

class CentralizedPeer : PeerBase {
    private:
        conn_t index_server;
    public:
        CentralizedPeer(conn_t index_server);
        int register_user();
        int register_directory(std::string);
        int register_file(std::string);
        int request_file(std::string);
        int search_for_file(std::string);
};

class DecentralizedPeer : PeerBase {
    private:
        FileIndex file_index;
        std::unordered_map<int, conn_t> received_queries;
        std::vector<conn_t> neighbors;

        void broadcast_query(conn_t sender, msg_t message);
        void backtrace_response(msg_t message);
    public:
        DecentralizedPeer(std::string neighbor_file) : PeerBase() {};
        int request_file(std::string);
        int search_for_file(std::string);
};