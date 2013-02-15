#ifndef MAC_ADDR_HH
#define MAC_ADDR_HH

#include <string>

class MACAddress
{
private:
  uint8_t octets[ 6 ];

  bool is_broadcast( void ) const;

public:
  MACAddress( const std::string & s_addr );
  bool matches( const MACAddress & other ) const;

  static std::string parse_human( const std::string & with_colons );
  std::string pp( void ) const;
};

#endif
