#include <assert.h>

#include "rate-estimate.hh"

RateEstimate::RateEstimate( const double s_empty_rate_estimate, const unsigned int s_averaging_extent_ms )
  : empty_rate_estimate( s_empty_rate_estimate),
    averaging_extent_ms( s_averaging_extent_ms ),
    history()
{
}

void RateEstimate::add_packet( const Payload & p )
{
  history.push( p );
  housekeeping();
}

double RateEstimate::get_rate( void )
{
  housekeeping();
  const int num_packets = history.size();
  if ( num_packets <= 2 ) {
    return empty_rate_estimate;
  }

  return 1000.0 * (double) num_packets / (double) averaging_extent_ms;
}

void RateEstimate::housekeeping( void )
{
  const uint64_t now = Socket::timestamp();
  while ( !history.empty() ) {
    Payload & front = history.front();
    assert( now >= front.recv_timestamp );
    if ( now - front.recv_timestamp > 1000000 * averaging_extent_ms ) {
      history.pop();
    } else {
      return;
    }
  }
}
