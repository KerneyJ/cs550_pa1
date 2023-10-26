# Download the base image
lxc image copy ubuntu:20.04 local: --alias custom-image

# Launch a temporary container for customization
lxc launch custom-image temp-container

# Resize the root disk to 10GB (for example)
# lxc config set temp-container volatile.rootfs.size 10GB
lxc config set temp-container limits.cpu 1
lxc config set temp-container limits.memory 6GB

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

$(pwd)/get-ips.sh

# Control the containers
for i in $(seq 1 $num_containers); do
  container_name="vm$i"
  lxc file push $(pwd)/peer-ips.csv $container_name/home/root/cs550_pa1/config/
  eval "lxc exec $container_name -- /home/root/cs550_pa1/scripts/gendata.sh $i /home/root/cs550_pa1" &
  lxc exec $container_name -- bash -lc "echo ''ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQCbon7ZO8PzmgB8zIMRxtius+suMFqQ0AKpVqgatew5FrzwCYdF204UzR/PM1bkLl23L9t0tzJ0XVYevjVLiggbfC15knHBhgOJXKxKhsFwc2K6u5UUazxJ4y6V9lL6cnho62Fx1HXQ0ZMPWPwAA3pKLEWZi47vOGYuxtKX8qgYNxkVI1WpNBoduYjDHZ/eq3w2dPrrtxBkr9h2hVQZuuxOFIC4b2Xx4jakP4NZHsOmQAc5UBuTQ5j9jBu/lkPiqFsjXCgR6233wULeEmqoMu7qp+r6lbx9SKVvuwY0hl5pMMlX0DaUyzn3cnspncKeKSZLC4AyTOpfBgRS+0ii02knoEkmu+2e317iVZ1/zk+sK1SQNIIC4rJhUk4nee+VvyUHUyMSl+yoTEthsRdp5WOLqaSUzBPX27Krm2FLCTtuVQyYN1Vt+lI+n9JqcZ5eVCpJl06AZkvE32XcofcDp9+cr47wblpwhRHIB8IHazUcIED9oDHMbuKv8wqirfIBdqk= cc@jaiaha > /root/.ssh/authorized_keys && chmod 644 /root/.ssh/authorized_keys"
  lxc exec vm$i -- bash -lc "cd /home/root/cs550_pa1 && echo '\$1 $i \$2 \$3' > ./scripts/start.sh && chmod +x ./scripts/start.sh"
done

