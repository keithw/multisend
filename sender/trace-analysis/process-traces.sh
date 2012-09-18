#! /bin/bash
# determine the number of sessions to extract the trace for .
ls --format=single-column client-*-* | cut -f 2,3 -d "-" > sessions.list
server_logs=`ls --format=single-column server-*`
echo "Server logs are "$server_logs
server_time=`echo $server_logs | cut -f 2 -d "-"`
echo "Server time is "$server_time
for session in `cat sessions.list` ;
do 
   session_name=`echo $session | cut -f 1 -d "-"`
   session_id=`echo $session | cut -f 2 -d "-"`
   echo "Session name is "$session_name;
   echo "Session id is "$session_id;

   # get a tmp server file 
   grep "senderid=$session_id" $server_logs > server-$session_name-$session_id

   # prepare the files for simulation
   nice -n 19 python prep-for-simulation.py client-$session_name-$session_id $session_id ;
   nice -n 19 python prep-for-simulation.py server-$session_name-$session_id $session_id ;

   # remove the tmp server file
   rm server-$session_name-$session_id ;
  
done
