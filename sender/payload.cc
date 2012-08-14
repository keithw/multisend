#include <assert.h>

#include "payload.hh"

const std::string Payload::str( const size_t len ) const
{
  assert( len >= sizeof( Payload ) );
  std::string padding( len - sizeof( Payload ), 0 );
  return std::string( (char*)this, sizeof( Payload ) ) + padding;
}

bool Payload::operator==( const Payload & other ) const
{
  return (sequence_number == other.sequence_number
	  && sent_timestamp == other.sent_timestamp
	  && recv_timestamp == other.recv_timestamp
	  && process_id == other.process_id);
}
