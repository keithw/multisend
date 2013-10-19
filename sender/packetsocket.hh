#ifndef PACKETSOCKET_HH
#define PACKETSOCKET_HH

#include <string>
#include <vector>
#include "macaddr.hh"

class PacketSocket
{
private:
  int sock;

  int get_index( const std::string & name ) const;

public:
  PacketSocket( const std::string & s_interface );

  std::vector< std::string > recv_raw( void );
  void send_raw( const std::string & input );
  int fd( void ) const { return sock; }
};

#endif
