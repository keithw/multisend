#!/bin/bash
trap 'finalize' 2

if [ $# -lt 5 ]; then
	   echo "Usage : $0 up_trace down_trace lossrate internet_facing_if client_facing_if uplink_log downlink_log"
	      exit
fi ;

BRIDGE_NAME="br0"
THROUGHPUT_GRAPH_RESOLUTION=100

UP_TRACE="$1"
DOWN_TRACE="$2"
LOSSRATE="$3"
INTERNET_IF="$4"
CLIENT_IF="$5"

UPLOG_SET=false
DOWNLOG_SET=false

if [ -z "$6" ]
then
	UPLINK_LOG=""
else	
	UPLOG_SET=true
	UPLINK_LOG="$6"
fi

if [ -z "$7" ]
then
	DOWNLINK_LOG=""
else	
	DOWNLOG_SET=true
	DOWNLINK_LOG="$7"
fi


finalize(){
	echo "# bringing bridge \"$BRIDGE_NAME\" up"
  ifconfig "$BRIDGE_NAME" up
	
	if [ "$UPLOG_SET" = true ]
	then
		echo "# generating graphs for uplink from log $UPLINK_LOG..."
		mm-delay-graph "$UPLINK_LOG" > $(dirname "${UPLINK_LOG}")/delay-graph-up.svg
		mm-throughput-graph $THROUGHPUT_GRAPH_RESOLUTION "$UPLINK_LOG" > $(dirname "${UPLINK_LOG}")/throughput-graph-up.svg
	fi

	if [ "$DOWNLOG_SET" = true ]
	then
		echo "# generating graphs for downlink from log $DOWNLINK_LOG..."
		echo $(dirname "${DOWNLINK_LOG}")/throughput-graph-down.svg
		mm-delay-graph "$DOWNLINK_LOG" > $(dirname "${DOWNLINK_LOG}")/delay-graph-down.svg
		mm-throughput-graph $THROUGHPUT_GRAPH_RESOLUTION "$DOWNLINK_LOG" > $(dirname "${DOWNLINK_LOG}")/throughput-graph-down.svg
	fi
}

ifconfig "$BRIDGE_NAME" down
sender/cellsim-setup.sh "$CLIENT_IF" "$INTERNET_IF"
sender/cellsim "$UP_TRACE" "$DOWN_TRACE" "$LOSSRATE" "$INTERNET_IF" "$CLIENT_IF" $UPLINK_LOG $DOWNLINK_LOG

#finalize

