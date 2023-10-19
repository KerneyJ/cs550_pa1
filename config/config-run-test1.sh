#!/bin/bash

#Currently just starts up the centralized network on the 17 vms.

#Expects two arguments: network topology choice, and test choice.
#Topology choice: 1: centralized, 2: star topology, 3: 2-D mesh topology
#Test choice: 1: query latency, 2: query throughput, 3: Transfer throughput small, 4: Transfer throughput large
#(See PA2 instructions for descriptions of the tests.)

#If centralized, initiatialize centralized topology

#Launch index server on peer 17
container_name="my-container-17"
lxc exec "$container_name" -- /bin/bash -l -c "/home/cc/cs550_pa1/bin/index_server"

# Use the lxc info command to retrieve the container's IP address
index_ip=$(lxc info "$container_name" | grep "eth0:" | awk '{print $2}')

# Display the IP address
echo "IP address of index server on $container_name is $index_ip"

#Launch peer server on peers 1 through 16.
num_containers=16
for i in $(seq 1 $num_containers); do
  container_name="my-container-$i"

  # Replace the following line with the command you want to run within each container
  lxc exec "$container_name" -- /bin/bash -l -c "/home/cc/cs550_pa1/bin/c_peer $index_ip"



#else if star, initializae star topology

#Launch peer servers on peers 1 through 16 



#else if mesh, initialize mesh topology



#If test 1 (query latency): 
#peer 1 requests 10k random files from small dataset.




#End tests 
#Close the servers on each node. 