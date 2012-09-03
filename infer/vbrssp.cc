#include <vector>
#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "receiver.hh"
#include "cellqueue.hh"

using namespace std;

int main( void )
{
  Receiver *receiver = NULL;
  CellQueue queue;
  std::queue< DeliveryForecast > forecasts;
  std::queue< unsigned int > received_packet_feedback;

  double time;
  const double TICK_LENGTH = 0.02;
  const unsigned int FEEDBACK_DELAY_TICKS = 2;
  const unsigned int TARGET_DELAY_TICKS = 5;

  int packets_received = 0;
  int packets_sent = 0;

  while ( 1 ) {
    if ( !cin.good() ) {
      exit( 0 );
    }

    int ms = -1;
    cin >> ms;

    double packet_time = (double) ms / 1000.0;

    if ( !receiver ) {
      receiver = new Receiver( packet_time );
      time = packet_time;
      for ( unsigned int i = 0; i < FEEDBACK_DELAY_TICKS; i++ ) {
	forecasts.push( receiver->forecast() );
	received_packet_feedback.push( 0 );
      }
    }

    if ( packet_time < time ) {
      if ( queue.recv( time ) ) {
	receiver->recv();
	packets_received++;
      }
    } else {
      /* need to advance time */
      while ( packet_time >= time ) {
	time += TICK_LENGTH;
	receiver->advance_to( time );
	DeliveryForecast forecast( forecasts.front() );
	forecasts.pop();
	forecasts.push( receiver->forecast() );
	assert( forecasts.size() == FEEDBACK_DELAY_TICKS );
	
	unsigned int delayed_packets_received( received_packet_feedback.front() );
	received_packet_feedback.pop();
	received_packet_feedback.push( packets_received );
	assert( received_packet_feedback.size() == FEEDBACK_DELAY_TICKS );

	int current_queue_size_estimate = packets_sent - delayed_packets_received - forecast.counts[ FEEDBACK_DELAY_TICKS ];

	if ( current_queue_size_estimate < 0 ) {
	  current_queue_size_estimate = 0;
	}

	/* send packets */
	int cumulative_delivery_forecast = forecast.counts[ TARGET_DELAY_TICKS ];

	int packets_to_send = cumulative_delivery_forecast - current_queue_size_estimate;

	printf( "%f, sent = %d, dpr = %d, fc = %d, cur est = %d, cum dev forecast = %d\n",
		time, packets_sent, delayed_packets_received, forecast.counts[ FEEDBACK_DELAY_TICKS ], current_queue_size_estimate,
		cumulative_delivery_forecast );

	if ( packets_to_send > 0 ) {
	  printf( "%f: sending %d packets\n",
		  time, packets_to_send );
	  for ( int i = 0; i < packets_to_send; i++ ) {
	    queue.send( time );
	    packets_sent++;
	  }
	}
      }
      
      if ( queue.recv( time ) ) {
	receiver->recv();
	packets_received++;
      }
    }
  }
}
