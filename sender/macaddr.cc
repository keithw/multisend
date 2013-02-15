#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include "macaddr.hh"

MACAddress::MACAddress( const std::string & s_addr )
  : octets()
{
  assert( s_addr.size() == 6 );
  for ( int i = 0; i < 6; i++ ) {
    octets[ i ] = s_addr[ i ];
  }
}

bool MACAddress::is_broadcast( void ) const
{
  for ( int i = 0; i < 6; i++ ) {
    if ( octets[ i ] != 0xff ) {
      return false;
    }
  }
  return true;
}

bool MACAddress::matches( const MACAddress & other ) const
{
  return ( is_broadcast() || other.is_broadcast() || (0 == memcmp( octets, other.octets, 6 )) );
}

std::string MACAddress::parse_human( const std::string & with_colons )
{
  std::string ret( 6, 0 );

  unsigned int octets[ 6 ];

  if ( with_colons.empty() ) {
    for ( int i = 0; i < 6; i++ ) {
      octets[ i ] = 0xff;
    }
  } else {
    int items_matched = sscanf( with_colons.c_str(),
				"%x:%x:%x:%x:%x:%x",
				&octets[ 0 ],
				&octets[ 1 ],
				&octets[ 2 ],
				&octets[ 3 ],
				&octets[ 4 ],
				&octets[ 5 ] );
    assert( items_matched == 6 );
  }

  for ( int i = 0; i < 6; i++ ) {
    assert( octets[ i ] <= 255 );
    ret[ i ] = octets[ i ];
  }

  return ret;
}

std::string MACAddress::pp( void ) const
{
  char tmp[ 64 ];
  snprintf( tmp, 64, "%x:%x:%x:%x:%x:%x",
	    octets[ 0 ],
	    octets[ 1 ],
	    octets[ 2 ],
	    octets[ 3 ],
	    octets[ 4 ],
	    octets[ 5 ] );
  return std::string( tmp );
}
