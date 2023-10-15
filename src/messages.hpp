#include <string>

extern "C" {
    #include "comms.h"
}

int send_once(conn_t conn, msg_t message);
msg_t send_and_recv(conn_t conn, msg_t message);

msg_t conn_to_msg(conn_t conn, msg_type);
msg_t str_and_conn_to_msg(std::string str, conn_t conn, msg_type);
msg_t str_to_msg(std::string str, conn_t conn, msg_type);

conn_t msg_to_conn(msg_t);
std::string msg_to_str(msg_t);
std::pair<std::string, conn_t> msg_to_str_and_conn(msg_t);