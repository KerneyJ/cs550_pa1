#!/bin/bash

#Expects two arguments: network topology choice, and test choice.
#Topology choice: 1: centralized, 2: star topology, 3: 2-D mesh topology
#Test choice: 1: query latency, 2: query throughput, 3: Transfer throughput small, 4: Transfer throughput large
#(See PA2 instructions for descriptions of the tests.)

function kill_peers()
{
  echo "Killing peers..."
  parallel-ssh -O StrictHostKeyChecking=no -h host-file.txt -t 0 'pkill _peer'
  exit
}
trap kill_peers SIGINT

log_file="/home/root/test_output_log.txt"

#If arg 1 == 1, initiatialize centralized topology
if [ "$1" = "1" ]; then
  echo "initializing centralized topology"
  container_name="vm17"
  # Use the lxc info command to retrieve the container's IP address
  index_ip=$(sudo lxc exec "$container_name" -- ip a show scope global | grep -Po "inet \K[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*" | head -1) 
  #Launch index server on peer 17
  ssh root@$index_ip -i /home/cc/.ssh/id_rsa -o StrictHostKeyChecking=no "pkill index_server"
  ssh root@$index_ip -i /home/cc/.ssh/id_rsa -o StrictHostKeyChecking=no screen -d -m "bash -c \"cd /home/root/cs550_pa1; stdbuf -oL ./bin/index_server\""
  sleep 2
  # Display the IP address
  index_ip="${index_ip}:8000"
  echo "IP address of index server on $container_name is $index_ip"

  #Launch peer server on peers 1 through 16.
  parallel-ssh -O StrictHostKeyChecking=no -h host-file.txt -t 0 -o logs "/bin/bash -c \"cd /home/root/cs550_pa1; stdbuf -oL ./scripts/start.sh ./bin/c_peer $index_ip $2\""

#else if arg 1 == 2: initialize star topology
elif [ "$1" = "2" ]; then 
  echo "Iniitializing star topology"
  parallel-ssh -O StrictHostKeyChecking=no -h host-file.txt -t 0 'pkill _peer'
  parallel-ssh -O StrictHostKeyChecking=no -h host-file.txt -t 0 -o logs "/bin/bash -c \"cd /home/root/cs550_pa1; stdbuf -oL ./scripts/start.sh ./bin/d_peer ./config/star-topology.txt $2\""

#else if arg 1 == 3, initialize mesh topology
elif [ "$1" = "3" ]; then 
  echo "Initializing mesh topology"
  parallel-ssh -O StrictHostKeyChecking=no -h host-file.txt -t 0 'pkill _peer'
  parallel-ssh -O StrictHostKeyChecking=no -h host-file.txt -t 0 -o logs "/bin/bash -c \"cd /home/root/cs550_pa1; stdbuf -oL ./scripts/start.sh ./bin/d_peer ./config/grid-topology.txt $2\""

fi

kill_peers

echo "Testing finished!"
