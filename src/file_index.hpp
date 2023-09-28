#include <unordered_map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

extern "C" {
    #include "comms.h"
}

class FileIndex {
    private:
        std::unordered_map<std::string, std::vector<conn_t>> file_peer_map;
        std::mutex index_lock;
    public:
        FileIndex();
        int replication_factor;
        int add_peer(std::string filename, conn_t peer); // returns number of peers
        conn_t get_rand_peer(std::string filename); // returns random peer that has registered this file for load balancing
        bool contains_peer(std::string filename, conn_t peer);  
        int count_peers(std::string filename);
};