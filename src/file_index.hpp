#include "comms.h"
#include <unordered_map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

class FileIndex {
    private:
        std::unordered_map<std::string, std::pmr::vector<conn_t>> file_peer_map;
        std::unordered_map<std::string, std::mutex> file_lock_map;
    public:
        FileIndex();
        void add_peer(std::string filename, conn_t peer);
        // returns random peer that has registered this file for load balancing
        conn_t get_peer(std::string filename);  
        uint count_peers(std::string filename);
};