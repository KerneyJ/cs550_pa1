#include "peer_base.hpp"
#include "server.hpp"
#include "thread_pool.hpp"

void message_handler(conn_t, msg_t) {}

int main(int, char**) {
    Server s = Server();
    s.start(message_handler, false);

    sleep(3);

    return 0;
}