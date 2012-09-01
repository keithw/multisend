#include <stdio.h>

#include "cellqueue.hh"

void CellQueue::send( const int time )
{
  _packets.push( time );
}

void CellQueue::recv( const int time )
{
  _opportunities++;

  if ( _packets.empty() ) {
    fprintf( stderr, "Underflow at time %d\n", time );
  } else {
    const int send_time = _packets.front();
    _packets.pop();
    _deliveries++;
    fprintf( stderr, "Packet delivered after delay of %d\n",
	     time - send_time );
  }
}
