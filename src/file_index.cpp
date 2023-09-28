#include "file_index.hpp"
#include "comms.h"
#include <unordered_map>
#include <mutex>
#include <string>
#include <vector>

FileIndex::FileIndex() {
}

int FileIndex::add_peer(std::string filename, conn_t peer) {
    std::unique_lock<std::mutex> mutex_lock (index_lock);
    auto peer_iter = file_peer_map.find(filename);

    // no lock found, no files in map
    if(peer_iter == file_peer_map.end()) {
        file_peer_map.emplace(filename, std::vector<conn_t> { peer });
        return 1;
    }
    
    file_peer_map[filename].push_back(peer);
    return file_peer_map[filename].size();
}

int FileIndex::count_peers(std::string filename) {
    std::unique_lock<std::mutex> mutex_lock (index_lock);
    auto peer_iter = file_peer_map.find(filename);

    // no lock found, no files in map
    if(peer_iter == file_peer_map.end())
        return 0;
    
    // lock found, there's already an entry for this filename
    return peer_iter->second.size();
}

conn_t FileIndex::get_rand_peer(std::string filename) {
    std::unique_lock<std::mutex> lock (index_lock);
    auto peer_iter = file_peer_map.find(filename);

    if(peer_iter == file_peer_map.end())
        return { -1, -1, -1 };
    
    auto peers = peer_iter->second;
    uint index = rand() % peers.size();
    return peers.at(index);
}

bool FileIndex::contains_peer(std::string filename, conn_t peer) {
    std::unique_lock<std::mutex> lock (index_lock);
    auto peer_iter = file_peer_map.find(filename);

    // TODO: this should not be hit in our use case, is this the right failure mode?
    // no lock found, no files in map
    if(peer_iter == file_peer_map.end())
        return false;

    for(auto p : peer_iter->second) {
        if(p.addr == peer.addr && p.port == peer.port && p.sock == peer.sock) {
            return true;
        }
    }
    
    return false;
}