#include <assert.h>

#include "history.hh"

History::History()
  : _outstanding(),
    _num_outstanding( 0 ),
    _num_lost( 0 )
{
}

void History::packet_sent( const Payload & p )
{
  _outstanding.push_back( p );
  _num_outstanding++;
}

void History::packet_received( const Payload & p )
{
  _outstanding.remove_if( [&] ( const Payload & x ) { return x.sequence_number == p.sequence_number; } );

  unsigned int new_size = _outstanding.size();
  assert( _num_outstanding - new_size == 1 );
  _num_outstanding = new_size;

  const uint64_t purge_older = p.sent_timestamp - reorder_window;

  _outstanding.remove_if( [&] ( const Payload & x ) { return x.sent_timestamp < purge_older; } );

  new_size = _outstanding.size();
  _num_lost += _num_outstanding - new_size;

  _num_outstanding = new_size;
}
