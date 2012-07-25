#include "history.hh"

History::History()
  : _outstanding(),
    _num_outstanding( 0 )
{
}

void History::packet_sent( const Payload & p )
{
  _outstanding.push_back( p );
  _num_outstanding++;
}

void History::packet_received( const Payload & p )
{
  const uint64_t purge_older = p.sent_timestamp - reorder_window;

  _outstanding.remove_if( [&] ( const Payload & x ) { return x.sent_timestamp < purge_older; } );

  _num_outstanding = _outstanding.size();
}
