#! /bin/bash
if [ $# -lt 1 ]; then
  echo "Usage : enter file name "
  exit;
fi
file=$1
export expt=`echo $file | cut -d '/' -f 3`
echo "Expt name is $expt"
rm *.cap *.delay
grep downlink $file | grep "/" > downlink.cap
grep uplink $file | grep "/" > uplink.cap
grep downlink $file | grep "delivery" > downlink.delay
grep uplink $file | grep "delivery" > uplink.delay
export t_offset=0
echo "Time offset is $t_offset"
gnuplot -p plot.p
mkdir $expt
mv uplink-plot.png $expt/uplink-$expt.png
mv downlink-plot.png $expt/downlink-$expt.png
mv *.cap *.delay $expt
cp $file $expt
