#! /bin/bash
# netem setup script for 30ms and 10Mbps with infinite buffer
sudo tc qdisc del dev lo root netem
sudo tc qdisc add dev lo root handle 1:0 netem delay 30ms
sudo tc qdisc add dev lo parent 1:1 handle 10: tbf rate 10Mbit burst 1000000000000 limit 300000000000000
