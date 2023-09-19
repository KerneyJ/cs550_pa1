# P2P File sharing service


Peer (* := user-facing): 
- * register_file(filename)
- * register_dir(filename)
- * search(filename)
- req-file(filename)
- * search_and_req_file(filename)   # calls search() then gets the IP of file owner from index server
- res_file(filename)                # send a file to a peer who requests it.