#ifndef SOCKET_HH
#define SOCKET_HH

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

class Socket {
public:
  class Address {
  private:
    struct sockaddr_in _sockaddr;

  public:
    Address( std::string ip, uint16_t port );
    Address( const struct sockaddr_in s_sockaddr ) : _sockaddr( s_sockaddr ) {}

    const struct sockaddr_in & sockaddr( void ) const { return _sockaddr; }
    const std::string str( void ) const;
  };

  class Packet {
  public:
    Address addr;
    std::string payload;

    Packet( const Address &s_addr, const std::string &s_payload )
      : addr( s_addr ), payload( s_payload )
    {}
  };

private:
  int sock;

public:
  Socket();
  void bind( const Address & addr ) const;
  void send( const Packet & payload ) const;
  void bind_to_device( const std::string & name ) const;
  Packet recv( void ) const;
};

#endif
