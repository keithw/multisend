#include <stdio.h>

#include "cellqueue.hh"

void CellQueue::send( const double time )
{
  _packets.push( time );
}

bool CellQueue::recv( const double time )
{
  _opportunities++;

  if ( _packets.empty() ) {
    fprintf( stderr, "Underflow at time %f\n", time );
    return false;
  } else {
    const double send_time = _packets.front();
    _packets.pop();
    _deliveries++;
    fprintf( stderr, "%f : packet delivered after delay of %f\n",
	     time, time - send_time );
    return true;
  }
}
