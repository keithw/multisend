#include <vector>
#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "receiver.hh"

using namespace std;

int main( void )
{
  Receiver *receiver = NULL;

  //  const double FRAME_INTERVAL = 0.1;
  //  double last_frame = -1;

  double time;
  const double TICK_LENGTH = 0.02;

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
    }

    if ( packet_time < time ) {
      receiver->recv();
    } else {
      /* need to advance time */
      while ( packet_time >= time ) {
	time += TICK_LENGTH;
	receiver->advance_to( time );
	DeliveryForecast forecast( receiver->forecast() );
	printf( "%.2f", time );
	for ( auto it = forecast.counts.begin();
	      it != forecast.counts.end();
	      it++ ) {
	  printf( " %d", *it );
	}
	printf( "\n" );
      }
      
      receiver->recv();
    }
  }
}
