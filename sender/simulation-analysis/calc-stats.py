#! /usr/bin/python
import sys
if (len(sys.argv) < 3) :
   print sys.argv[0]," file stat";
   exit(4)

stat=sys.argv[2];
file_name=open(sys.argv[1],"r")
bytes_sent=0
bytes_avail=0
start_time=-1
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
  print (100*float(bytes_sent))/float(bytes_avail),"%"

sum_delay=0;
count=0;
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
     count=count+1;
  print float(sum_delay)/float(count),"ms"
