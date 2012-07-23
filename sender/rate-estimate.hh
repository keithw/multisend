#ifndef RATE_ESTIMATE_HH
#define RATE_ESTIMATE_HH

#include <queue>

#include "socket.hh"
#include "payload.hh"

class RateEstimate
{
private:
  const double empty_rate_estimate; /* packets per second */
  const unsigned int averaging_extent_ms;
  std::queue< Payload > history;

  void housekeeping( void ); /* cull packets older than averaging_extent_ms */

public:
  RateEstimate( const double s_empty_rate_estimate, const unsigned int s_averaging_extent_ms );
  void add_packet( const Payload & p );
  double get_rate( void ); /* rate in packets per second */
};

#endif
