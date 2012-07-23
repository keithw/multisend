#include <assert.h>

#include "payload.hh"

const std::string Payload::str( const size_t len ) const
{
  assert( len >= sizeof( Payload ) );
  std::string padding( len - sizeof( Payload ), 0 );
  return std::string( (char*)this, sizeof( Payload ) ) + padding;
}
