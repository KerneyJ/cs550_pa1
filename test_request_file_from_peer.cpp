#include "peer.hpp"
#include <stdio.h>
extern "C" {
    #include "comms.h"
}

/* 
The problem: peer is failing to get file from other hosts.

On host1, peer sends a file request via request_file_from_peer() to host2.
Then, if host 2 has the file, host2 sends the file to host1 via send_file().
Then host 1 writes file to ./data directory.
At this point, host1 should have the file and should call register_file() to update the index server.


To test, you need to launch a host2 and then in another shell run ./test_request_file_from_peer
*/

//Step 1: send file request to host 2. Confirm that host 2 gets the file request.
int main(int argc, char const *argv[])
{

    char *ip = "127.0.0.1";
    int port = 8081;
    char filename[] = "nice_file_to_have";
    conn_t conn = {0, 0, 0};
    msg_t message;
    msg_t reply;
    create_message(&message, filename, REQUEST_FILE);
    clntinit_conn(&conn, ip, port);
    
    send_msg(message, conn); 
    
    reply = recv_msg(conn); 
    delete_msg(&message);
    close_conn(&conn);

    

    return 0;
}





//Step 2: send file to host 1. Confirm that host 1 got file.

//Step 3: confirm that host1 wrote the file to ./data dir

//Step 3: register file with index server. Confirm that index server has registered the file.