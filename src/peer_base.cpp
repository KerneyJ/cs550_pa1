#include "peer_base.hpp"
#include "comms.h"

static conn_t new_connection() {
	struct sockaddr_in serv_addr;
    int ip, port, sock;

    if(get_ipv4_address(&ip) < 0)
        return {-1, -1, -1};
    
	sock = socket(AF_INET, SOCK_STREAM, 0);
    for(port = 8000; port < 9000; port++) {
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = port;
        serv_addr.sin_addr.s_addr = ip;

    	if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            // failed to bind, check next port
            continue;
        }

        close(sock);

        return {ip, port, -1};
    }

    return {-1, -1, -1};
}

PeerBase::PeerBase() {
    
}

int main(int, char**) {
    const int n_conns = 10;
    conn_t conn_info;
    conn_t conns[n_conns];

    for(int i = 0; i < n_conns; i++) {
        conn_info = new_connection();
        servinitco_conn(&conns[i], &conn_info);
        printf("New connection created! {%d, %d, %d}\n", conns[i].addr, conns[i].port, conns[i].sock);
    }

    for(int i = 0; i < n_conns; i++) {
        printf("Closing connection, {%d, %d, %d}\n", conns[i].addr, conns[i].port, conns[i].sock);
        close_conn(&conns[i]);
    }
}