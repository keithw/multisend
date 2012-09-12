set term png size 1920,1080
set xlabel "Time in seconds"
set output "uplink-plot.png"
set xrange[0:*]
set multiplot layout 2,1 title "Uplink - `echo $expt` "
set ylabel "Rate in Mbps"
time_offset=`echo $t_offset`
set title "Uplink throughput"
set lmargin 15
set style data lines
plot  "./uplink.cap" u ($2-time_offset):($5*8/1.0e6) title "available", "./uplink.cap" u ($2-time_offset):($9*8/1.0e6) title "attempted to use"
set ylabel "Delay in seconds"
set lmargin 15

set style data dots
set logscale y
set title "Uplink delay"
plot "./uplink.delay" u ($2-time_offset):($4/1.0e3) title "delay",0.15 title "150 ms"
unset logscale y

unset multiplot
set output "downlink-plot.png"
set multiplot layout 2,1 title "Downlink - `echo $expt` "
set ylabel "Rate in Mbps"
set title "Downlink throughput"
set lmargin 15
set style data lines
plot  "./downlink.cap" u ($2-time_offset):($5*8/1.0e6) title "available","./downlink.cap" u ($2-time_offset):($9*8/1.0e6) title "attempted to use"  
set ylabel "Delay in seconds"

set style data dots
set logscale y
set title "Downlink delay"
set lmargin 15
plot "./downlink.delay" u ($2-time_offset):($4/1.0e3) title "delay",0.15 title "150 ms"
unset logscale y
