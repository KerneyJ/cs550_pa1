# P2P File sharing service

### Description
P2P file service implemented as a class project for IIT CS550, Advanced OS (Distributed Systems).

Allows peers to request and send files to each other over a network and query an index server to search for avialable files.
Supports data resilience by replicating files among multiple peers. Number of replications can be configured system-wide on the index server.


### Instructions to build and run

1. Clone the repository by running `git clone https://github.com/KerneyJ/cs550_pa1`
2. Run `mkdir ./bin ./data`
3. To compile, run `make all`, or if you'd like to see lots of debug messages in your shell, run `make DEBUG=true`. Performance wil be worse because of all the printf statements.
4. Launch the index server by running `./bin/index_server`
5. Decide how many peers you want to launch. For each peer, launch a new shell.
6. Make a data sub-directory for each peer. So for peer 1, run `mkdir ./data/data_peer1`, for peer 2, run `mkdir ./data/data_peer2`, etc. 
7. Populate the data directories with some test files for each peer. 
8. Launch the peers. For each peer, in its respective shell, run `./bin/peer_server 127.0.0.1 {port#}`, where port# is 8081 for peer 1, 8082 for peer 2, etc.
9. To interact with a peer via a CLI, open a new shell and run `./peer_CLI`. You will be prompted with instructions to connect the CLI with a corresponding peer_server of your choice.
10. Use the CLI to interact with the system. You can verify data transfers by inspecting the corresponding data directories.

