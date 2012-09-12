#! /bin/bash
if [ $# -lt 3 ] ; then
    echo "Enter folders one each for skype sprout and cubic"
    exit
fi
skype=$1
sprout=$2
cubic=$3
set -v
python calc-stats.py $skype/uplink.cap util
python calc-stats.py $skype/uplink.delay delay
python calc-stats.py $sprout/uplink.cap util
python calc-stats.py $sprout/uplink.delay delay
python calc-stats.py $cubic/uplink.cap util
python calc-stats.py $cubic/uplink.delay delay

python calc-stats.py $skype/downlink.cap util
python calc-stats.py $skype/downlink.delay delay
python calc-stats.py $sprout/downlink.cap util
python calc-stats.py $sprout/downlink.delay delay
python calc-stats.py $cubic/downlink.cap util
python calc-stats.py $cubic/downlink.delay delay
