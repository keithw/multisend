#! /bin/bash
# stuff going towards the server on the egress after going through the sim
tcpdump -r sprout-vs-sprout-eth0.pcap -tt -l -e  dst host 128.30.77.65 and dst port 60001 and udp > uplink.60001
tcpdump -r sprout-vs-sprout-eth0.pcap -tt -l -e  dst host 128.30.77.65 and dst port 60002 and udp > uplink.60002
# stuff going towards the client on the ingress after going through the sim
tcpdump -r sprout-vs-sprout-eth1.pcap -tt -l -e  src host 128.30.77.65 and src port 60001 and udp > downlink.60001
tcpdump -r sprout-vs-sprout-eth1.pcap -tt -l -e  src host 128.30.77.65 and src port 60002 and udp > downlink.60002

python process-traces.py uplink.60001 > uplink.60001.rate
python process-traces.py uplink.60002 > uplink.60002.rate
python process-traces.py downlink.60001 > downlink.60001.rate
python process-traces.py downlink.60002 > downlink.60002.rate

gnuplot -p plot.p
