set term png size 1920,1080
set xlabel "Time in seconds"
set output "tpt.png"
set multiplot layout 2,1 title "Skype vs Cubic vs Sprout - throughput "
skype_offset=1347419342
cubic_offset=1347412742
sprout_offset=1347414100
set style data lines
set ylabel "Rate in Mbps"
set title "uplink throughput"
set lmargin 15
plot "./`echo $skype`/uplink.cap" u ($2-skype_offset):($9*8/1.0e6) title "skype-uplink" , "./`echo $skype`/uplink.cap" u ($2-skype_offset):($5*8/1.0e6) title "available", "./`echo $cubic`/uplink.cap" u ($2- cubic_offset):($9*8/1.0e6) title"cubic-uplink" , "./`echo $sproutbt`/uplink.cap" u ($2- sprout_offset):($9*8/1.0e6) title"sproutbt-uplink"


set title "downlink throughput"
set lmargin 15
plot "./`echo $skype`/downlink.cap" u ($2-skype_offset):($9*8/1.0e6) title "skype-downlink" , "./`echo $skype`/downlink.cap" u ($2-skype_offset):($5*8/1.0e6) title "available", "./`echo $cubic`/downlink.cap" u ($2- cubic_offset):($9*8/1.0e6) title"cubic-downlink", "./`echo $sproutbt`/downlink.cap" u ($2- sprout_offset):($9*8/1.0e6) title"sproutbt-downlink"


unset multiplot
set output "delay.png"
set multiplot layout 2,1 title "Skype vs Cubuc vs Sprout - delay "
set logscale y
set style data dots
set ylabel "Delay in seconds"
set title "uplink delay"
set lmargin 15
plot "./`echo $skype`/uplink.delay" u ($2-skype_offset):($4/1.0e3) title "skype-uplink"  ,  "./`echo $cubic`/uplink.delay" u ($2- cubic_offset):($4/1.0e3) title"cubic-uplink"  , "./`echo $sproutbt`/uplink.delay" u ($2- sprout_offset):($4/1.0e3) title"sproutbt-uplink" 


set title "downlink delay"
set lmargin 15
plot "./`echo $skype`/downlink.delay" u ($2-skype_offset):($4/1.0e3) title "skype-downlink"  , "./`echo $cubic`/downlink.delay" u ($2- cubic_offset):($4/1.0e3) title"cubic-downlink" , "./`echo $sproutbt`/downlink.delay" u ($2- sprout_offset):($4/1.0e3) title"sproutbt-downlink" 

