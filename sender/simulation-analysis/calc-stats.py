#! /usr/bin/python
import sys
import math
if (len(sys.argv) < 3) :
   print sys.argv[0]," file stat";
   exit(4)

stat=sys.argv[2];
file_name=open(sys.argv[1],"r")
bytes_sent=0
bytes_avail=0
start_time=-1
util_list=[]

def mean(samples) :
   return float(sum(samples))/len(samples)
def percentile(samples,number) :
   samples.sort()
   reqd_index=int(round(number*0.01*len(samples)))
   assert(reqd_index <= len(samples))
   return samples[reqd_index] 
if(stat=="util"):
  for line in file_name.readlines() :
   records=line.split()
   if ((records[3]=="/") and (records[7]=="%")) :
     if (start_time == -1) :
       start_time = int(float(records[1]));
     time=int(float(records[1]));
     if(time-start_time <= 60) :
        continue ;
        # ignore first 60 seconds
     bytes_sent+=int(records[2]);
     bytes_avail+=int(records[4]);
     if ( (int(records[2]) !=0) and (int(records[4]) !=0) ) :
        util_list.append(float(records[6]));
  print "mean ",(100*float(bytes_sent))/float(bytes_avail),"% "," 25th percentile  ",percentile(util_list,25),"%", " 75th percentile ",percentile(util_list,75)," %"

sum_delay=0;
count=0;
delay_list=[]
if(stat=="delay"):
  for line in file_name.readlines() :
   records=line.split()
   if (records[2]=="delivery") :
     if (start_time == -1) :
       start_time = int(float(records[1]));
     time=int(float(records[1]));
     if(time-start_time <= 60) :
        continue ;
        # ignore first 60 seconds
     sum_delay+=int(records[3]);
     delay_list.append(int(records[3]));
     count=count+1;
  print "mean ", float(sum_delay)/float(count),"ms"," 25th percentile  ",percentile(delay_list,25)," ms ", " 75th percentile ",percentile(delay_list,75)," ms "
