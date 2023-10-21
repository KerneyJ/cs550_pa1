# P2P File sharing service

### Authors
* Ian Dougherty idougherty@hawk.iit.edu
* Jamison Kerney jkerney@hawk.iit.edu
* Harrison Mohr jmohr1@hawk.iit.edu

## README for PA2 (Decentralized version)

We assume that you are testing this in an environment where you've arleady initialized 17 Linux VM's. 

To try out the system with a decentralized topology,  
On each VM:
1. git clone https://github.com/KerneyJ/cs550_pa1 
2. run `make` in the project top-level directory (same directory where the Makefile is) to compile the code.
3. Create some non-empty data files in one or more of the VM's so that you'll have something to search for and transfer.
4. Launch a decentralized peer. The launch script takes two arguments: peer #, topology config file. Two topology files are provided.
   Use the same topology file for every peer.
   Here's an example of how you'd launch peer 3 with the grid-topology.txt file from the project's top-level directory:
   `./decentralized_peer 3 ./config/grid-topology.txt`
5. After running the above command, a CLI will have launched and you can choose to search for and request files using the CLI.















## README for PA1 (Centralized version)
### Description
P2P file service implemented as a class project for IIT CS550, Advanced OS (Distributed Systems).

Allows peers to request and send files to each other over a network and query an index server to search for avialable files.
Supports data resilience by replicating files among multiple peers. Number of replications can be configured system-wide on the index server.


### Instructions to build and run

2. Run `mkdir ./bin ./data`
3. To compile, run `make all`, or if you'd like to see lots of debug messages in your shell, run `make DEBUG=true`. Performance wil be worse because of all the printf statements.
4. Launch the index server by running `./bin/index_server`
5. Decide how many peers you want to launch. For each peer, launch a new shell.
6. Make a data sub-directory for each peer. So for peer 1, run `mkdir ./data/data_peer1`, for peer 2, run `mkdir ./data/data_peer2`, etc. 
7. Populate the data directories with some test files for each peer. 
8. Launch the peers. For each peer, in its respective shell, run `./bin/peer_server 127.0.0.1 {port#}`, where port# is 8081 for peer 1, 8082 for peer 2, etc.
9. To interact with a peer via a CLI, open a new shell and run `./peer_CLI`. You will be prompted with instructions to connect the CLI with a corresponding peer_server of your choice.
10. Use the CLI to interact with the system. You can verify data transfers by inspecting the corresponding data directories.

