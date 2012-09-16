#! /usr/bin/python
import sys
if(len(sys.argv) < 3 ) : 
  print "Usage : prep-for-simulation.py <file-name> <session-number> "
  exit
file_name=sys.argv[1];
fh=open(file_name,"r");
session_number=int(sys.argv[2]);

#parse file_name to determine client vs server log_pos-timestamp-sessionID
log_details=file_name.split("-")
log_pos=log_details[0];

if(log_pos=="client") :
  log_session=int(log_details[2]);
  if(session_number != log_session) :
        print "Client log file session number doesn't match input session number ";
        exit
  else :
    output_handle=open("downlink-"+str(session_number)+".pps","w");
elif(log_pos=="server") :
  output_handle=open("uplink-"+str(session_number)+".pps","w");

start_time=-1
for line in fh.readlines() :
   fields=line.split();
   if(fields[0]=="OUTGOING") : 
      # only INCOMING packets for simulation
      continue ;
   elif (fields[0]=="INCOMING") :
      sender_id=int(fields[3].split("=")[1].strip(","))
      if(sender_id==session_number) :
        recv_time_ms=int(int(fields[6].split("=")[1].strip(","))/1.0e6)  
        if(start_time==-1) :
           start_time=recv_time_ms;
        output_handle.write(str(recv_time_ms-start_time)+"\n");
