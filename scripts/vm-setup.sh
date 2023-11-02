# Download the base image
lxc image copy ubuntu:20.04 local: --alias custom-image --vm

# Launch a temporary container for customization
lxc launch custom-image -c limits.cpu=1 -c limits.memory=6GB temp-vm

# Resize the root disk to 10GB (for example)
# lxc config set temp-container volatile.rootfs.size 10GB
# lxc config set temp-container limits.cpu 1
# lxc config set temp-container limits.memory 6GB

sleep 40

lxc exec temp-vm -- apt autoremove -yy 
lxc exec temp-vm -- apt update -yy
lxc exec temp-vm -- apt install make gcc g++ -yy
lxc exec temp-vm -- git clone https://github.com/KerneyJ/cs550_pa1.git /home/root/cs550_pa1
lxc exec temp-vm -- make -C /home/root/cs550_pa1

# Stop the vm
lxc stop temp-vm

# Publish the customized image
lxc publish temp-vm --alias custom-image

# Delete the temporary vm
lxc delete temp-vm
# Number of vm to create
num_vms=17

# Create and start the vms using the custom image
for i in $(seq 1 $num_vms); do
  vm_name="vm$i"
  lxc launch custom-image "$vm_name" -c limits.cpu=1 -c limits.memory=6GB 
done

# Wait for the vms to be ready
sleep 10

$(pwd)/get-ips.sh

# Control the vms
for i in $(seq 1 $num_vms); do
  vm_name="vm$i"
  lxc file push $(pwd)/peer-ips.csv $vm_name/home/root/cs550_pa1/config/
  eval "lxc exec $vm_name -- /home/root/cs550_pa1/scripts/gendata.sh $i /home/root/cs550_pa1" &
  lxc exec $vm_name -- bash -lc "echo ''ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQDWdSDGzsp8yGO+0cLnMtwsRLx/GAqJ93MKCa936o8HD9peKg6om1Jpi6aHPdaFVzAcz+1btO57+i7RXIz2G0YfaTLhWwEJ1xS+OMBhDaFeATYX7deZ17dKfzeD+jUNV5prs+axzq9RW0585bfEY70Vb0M7vWBsBP6AR96qXpPwjBV8nQViEViarkmoYMyUWNJKFerVLu/7Mn6vj1dkdwPmWYjJwCQstNucHot0pxHONoIVadVx5HpuFTg6fp2/+9WqWhIQ6WjLB36NYmWI2cfHdwVrVZQvdP1c6QNDTlD3r1yizTfbrrT0QMq1sQJTu8cS4zKLMjQru28Pqrs6+dfnS3Irpuzz/U/+9sI6PICekWJ0ay4NjgwHu3TuMTg2J3O5ACyidrOdiz9Z3hFfN2iecZ5Xf5sQjm7btlVnV9hDvuxd9FdgWzkmf9c8e6KhDSTRRSYCQW8D1FoN0gkEQqsSqa/qJOSV0LeTMIMjqOGAA7VouMubZhEgSddqIr1itjk= cc@idougherty > /root/.ssh/authorized_keys && chmod 644 /root/.ssh/authorized_keys"
  lxc exec vm$i -- bash -lc "cd /home/root/cs550_pa1 && echo '\$1 $i \$2 \$3' > ./scripts/start.sh && chmod +x ./scripts/start.sh"
done

