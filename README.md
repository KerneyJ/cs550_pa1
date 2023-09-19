# P2P File sharing service

Pseudocode Outline
--------------------

(Peer (U := user-facing))



- U register_file(filename)
- U register_dir(filename)
- U search(filename)
- req-file(filename)
- U search_and_req_file(filename)   # calls search() then gets the IP of file owner from index server
- res_file(filename)                # send a file to a peer who requests it.

Indexing Server: 
- register(ip, file)
    index.addUserIfNew(ip)
    index.registerFileWUser(ip, file)
    if(index.count(file) < Rep):            #Rep := replication number.
        rep_req(random_ip, file, ip)
        if(nobody else):
            requeue.add(ip, file)
    if(reqpqueue.notempty()):
        rep_file, rep_ip = repqueue.pop()
        rep-req(ip, rep_file, rep_ip)

- search(file):
    index.get(file)
    return random(ips)