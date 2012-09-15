#! /usr/bin/python
import sys
fh=open(sys.argv[1],"r");
start_time=-1
rate_estimate=[0]*1000
for line in fh.readlines() :
  current_time=float(line.split()[0]);
  bytes_rx=int(line.split()[12].strip(":"));
  if ( start_time == -1 ) :
    start_time = current_time
  assert((current_time-start_time) <= 1000);
  rate_estimate[int(round(current_time-start_time))] += bytes_rx*8

for i in range(0,len(rate_estimate)):
  print i,"\t",rate_estimate[i];

