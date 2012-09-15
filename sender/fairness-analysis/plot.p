set term png size 1920,1080
set output "fairness.png"
set xlabel "Time in seconds"
set ylabel "Rate in mbps"
set multiplot layout 2,1 title "Fairness with 2 sprout bt flows"
set title "uplink"
plot "uplink.60001.rate", "uplink.60002.rate"
set title "downlink"
plot "downlink.60001.rate", "downlink.60002.rate"
