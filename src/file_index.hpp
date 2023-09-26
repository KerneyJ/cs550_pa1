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
        std::unordered_map<std::string, std::mutex> file_lock_map;
    public:
        FileIndex();
        void add_peer(std::string filename, conn_t peer);
        // returns random peer that has registered this file for load balancing
        conn_t get_rand_peer(std::string filename);  
        bool contains_peer(std::string filename, conn_t peer);  
        uint count_peers(std::string filename);
};