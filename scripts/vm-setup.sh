# Download the base image
lxc image copy ubuntu:20.04 local: --alias custom-image

# Launch a temporary container for customization
lxc launch custom-image temp-container

# Resize the root disk to 10GB (for example)
# lxc config set temp-container volatile.rootfs.size 10GB
lxc config set temp-container limits.cpu 1
lxc config set temp-container limits.memory 6GB
lxc config device add temp-container root disk pool=temp-pool source=temp-vol path=/

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
  lxc launch custom-image "$container_name" -c security.privileged=true --storage=temp-pool
done

# Wait for the containers to be ready
sleep 10

$(pwd)/get-ips.sh

# Control the containers
for i in $(seq 1 $num_containers); do
  container_name="vm$i"
  # Replace the following line with the command you want to run within each container example: lxc exec "$container_name" -- /bin/bash -l -c "date"
  lxc file push $(pwd)/peer-ips.csv $container_name/home/root/cs550_pa1/config/
  eval "lxc exec $container_name -- /home/root/cs550_pa1/scripts/gendata.sh $i /home/root/cs550_pa1" &
  lxc exec $container_name -- bash -lc "echo 'ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQCdNM8SfW+mdZlXGnvm1jV6uRyvIJH8o3wN91tfvdtvvYs2i9YDuQXWC3R6THRJJ4zH8XXt/PPyL0JxzlSHZO3u4screCyOqj4bjbub6W+AygXgIrC2eg9ErcfcKiWbzbajhwvXmXU3mk/ayOiDUGwJG65o/NgV5N01Ump7fvs8dNRI9mSBwmSH7sWd0dmf7SDi6hH8akrx5/PpnRWqZRD9mSHmssUttPwjmQyQt0eHeuY+jAfRC2+4gxE9JEP3Wlg2PWufpUavHhLwXNDgypzawDv4zs3XSZTj5tzuWBFhU2VUTxzxTcFUohhVFfHYn8WOFJIPbGzuKgynQiAizGMAa7iDXtcX879sWGI5IHp/OOUqsDzmzPe37/EzmPXphFsha2hiZ+lBpCurG5Ui4qum3aBPbXfQi4kMSOFgjMajVNsQ3KNOmOWMYTQbrKkJr8YJ/d4cFidscMyU5zx3fRWIujX7ZQfYsS3lBZC2qbxeVe/3Ar/UXaV0TYHDudbbg0s= cc@jaiaha-pa2' > /root/.ssh/authorized_keys && chmod 644 /root/.ssh/authorized_keys"
  lxc exec vm$i -- bash -lc "cd /home/root/cs550_pa1 && echo '\$1 $i \$2 \$3' > ./scripts/start.sh && chmod +x ./scripts/start.sh"
done

