#include "thread_pool.hpp"
#include <array>
#include <signal.h>
#include <unordered_map>
#include <unordered_set>

extern "C" {
    #include "comms.h"
}

#include "server.hpp"
#include "file_index.hpp"

class IPeer
{
    public:
        virtual ~IPeer() {}
        virtual int register_user() = 0;
        virtual int register_directory(std::string) = 0;
        virtual int register_file(std::string) = 0;
        virtual int request_file(std::string) = 0;
        virtual conn_t search_for_file(std::string) = 0;
};

class CentralizedPeer : public IPeer {
    private:
        Server server = Server(false);;
        conn_t index_server;
        int request_file(conn_t, std::string);
        int send_file(conn_t, msg_t);
        int replicate_file(conn_t, msg_t);
        void message_handler(conn_t, msg_t);
    public:
        CentralizedPeer(conn_t index_server);
        int register_user();
        int register_directory(std::string);
        int register_file(std::string);
        int request_file(std::string);
        conn_t search_for_file(std::string);
};

class DecentralizedPeer : public IPeer {
    private:
        unsigned char peer_id;
        std::mutex message_id_lock;
        unsigned int message_count;
        Server server;
        std::unordered_set<std::string> file_set;
        std::mutex query_map_lock;
        std::unordered_map<int, conn_t> received_queries;
        std::vector<conn_t> neighbors;
        void broadcast_query(conn_t sender, msg_t message);
        void backtrace_response(conn_t sender, msg_t message);
        void send_file(conn_t, msg_t);
        void search_index(conn_t, msg_t);
        void message_handler(conn_t, msg_t);
        void init_neighbors();
        void init_fileset();
        int get_message_id();
    public:
        DecentralizedPeer(unsigned char peer_id);
        int register_user();
        int register_directory(std::string);
        int register_file(std::string);
        int request_file(std::string);
        conn_t search_for_file(std::string);
};