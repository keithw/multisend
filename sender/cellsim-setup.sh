#! /bin/bash
# cellsim setup

if [ $# -lt 2 ]; then
   echo "Usage : ./cellsim-setup.sh ingress egress"
   exit
fi ;

ingress=$1
egress=$2

# put both interfaces in promisc mode.
set -v
set -x
sudo ifconfig $ingress up promisc
sudo ifconfig $egress up promisc


# Disable segmentation offloading to NIC.

sudo ethtool --offload  $ingress gso off  tso off gro off
sudo ethtool --offload  $egress gso off  tso off gro off

