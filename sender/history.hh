#ifndef HISTORY_HH
#define HISTORY_HH

#include "payload.hh"

#include <list>

class History
{
private:
  static const uint64_t reorder_window = 20 * 1000 * 1000; /* 20 ms */

  std::list< Payload > _outstanding;
  int _num_outstanding;

public:
  History();
  void packet_sent( const Payload & p );
  void packet_received( const Payload & p );
  int num_outstanding( void ) const { return _num_outstanding; }
};

#endif
