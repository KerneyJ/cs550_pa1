# Download the base image
lxc image copy ubuntu:20.04 local: --alias custom-image

# Launch a temporary container for customization
lxc launch custom-image temp-container

# Resize the root disk to 10GB (for example)
# lxc config set temp-container volatile.rootfs.size 10GB

lxc exec temp-container -- apt autoremove -yy
lxc exec temp-container -- apt update -yy
lxc exec temp-container -- apt install make gcc g++ -yy
lxc exec temp-container -- git clone https://github.com/KerneyJ/cs550_pa1.git /home/root/cs550_pa1
lxc exec temp-container -- make -C /home/root/cs550_pa1

# Stop the container
lxc stop temp-container

# Publish the customized image
lxc publish temp-container --alias custom-image

# Delete the temporary container
lxc delete temp-container
# Number of containers to create
num_containers=17

# Create and start the containers using the custom image
for i in $(seq 1 $num_containers); do
  container_name="vm$i"
  lxc launch custom-image "$container_name" -c security.privileged=true
done

# Wait for the containers to be ready
sleep 10

$(pwd)/get_ips.sh
#echo "" > $(pwd)/peer_ips.csv
#for i in {1..19}; do
#	IP=$(lxc exec "vm$i" -- ip a show scope global | grep -Po "inet \K[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*" | head -1);
#	echo "$i,$IP" >> $(pwd)/peer_ips.csv
#done

# Control the containers
for i in $(seq 1 $num_containers); do
  container_name="vm$i"
  # Replace the following line with the command you want to run within each container example: lxc exec "$container_name" -- /bin/bash -l -c "date"
  lxc file push $(pwd)/peer_ips.csv $container_name/home/root/cs550_pa1/config/
  eval "lxc exec $container_name -- /home/root/cs550_pa1/scripts/gendata.sh $i /home/root/cs550_pa1" &
done

