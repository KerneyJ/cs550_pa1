#include "file_index.hpp"
#include "comms.h"
#include <unordered_map>
#include <mutex>
#include <string>
#include <vector>

FileIndex::FileIndex() {
}

void FileIndex::add_peer(std::string filename, conn_t peer) {
    auto lock_iter = file_lock_map.find(filename);

    // no lock found, no files in map, locking is unneccessary
    if(lock_iter == file_lock_map.end()) {
        file_peer_map.insert({ filename, { peer } });
        return;
    }
    
    // lock found, there's already an entry for this filename
    std::unique_lock<std::mutex> lock (lock_iter->second);
    file_peer_map.at(filename).push_back(peer);

}

uint FileIndex::count_peers(std::string filename) {
    auto lock_iter = file_lock_map.find(filename);

    // no lock found, no files in map
    if(lock_iter == file_lock_map.end())
        return 0;
    
    // lock found, there's already an entry for this filename
    std::unique_lock<std::mutex> lock (lock_iter->second);
    return file_peer_map.at(filename).size();
}

conn_t FileIndex::get_peer(std::string filename) {
    auto lock_iter = file_lock_map.find(filename);

    // no lock found, no files in map
    if(lock_iter == file_lock_map.end())
        return { -1, -1, -1 };
    
    // lock found, there's already an entry for this filename
    std::unique_lock<std::mutex> lock (lock_iter->second);

    auto peers = file_peer_map.at(filename);
    uint index = rand() % peers.size();
    return peers.at(index);
}