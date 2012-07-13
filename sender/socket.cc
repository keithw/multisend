#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>

#include "socket.hh"

using namespace std;

Socket::Address::Address( string ip, uint16_t port )
  : _sockaddr()
{
  _sockaddr.sin_family = AF_INET;
  _sockaddr.sin_port = htons( port );

  if ( inet_aton( ip.c_str(), &_sockaddr.sin_addr ) == 0 ) {
    fprintf( stderr, "Invalid IP address (%s)\n", ip.c_str() );
    exit( 1 );
  }
}

const string Socket::Address::str( void ) const
{
  char tmp[ 64 ];
  snprintf( tmp, 64, "%s:%d", inet_ntoa( _sockaddr.sin_addr ), ntohs( _sockaddr.sin_port ) );
  return string( tmp );
}

Socket::Socket()
  : sock( socket( AF_INET, SOCK_DGRAM, 0 ) )
{
  if ( sock < 0 ) {
    perror( "socket" );
    exit( 1 );
  }
}

void Socket::bind( const Socket::Address & addr ) const
{
  if ( ::bind( sock, (sockaddr *)&addr.sockaddr(), sizeof( addr.sockaddr() ) ) < 0 ) {
    fprintf( stderr, "Error binding to %s\n", addr.str().c_str() );
    perror( "bind" );
    exit( 1 );
  }
}

void Socket::send( const Socket::Packet & packet ) const
{
  ssize_t bytes_sent = sendto( sock, packet.payload.data(), packet.payload.size(), 0,
			       (sockaddr *)&packet.addr.sockaddr(), sizeof( packet.addr.sockaddr() ) );
  if ( bytes_sent != static_cast<ssize_t>( packet.payload.size() ) ) {
    perror( "sendto" );
  }
}

void Socket::bind_to_device( const std::string & name ) const
{
  if ( setsockopt( sock, SOL_SOCKET, SO_BINDTODEVICE, name.c_str(), name.size() ) < 0 ) {
    fprintf( stderr, "Error binding to %s\n", name.c_str() );
    perror( "setsockopt SO_BINDTODEVICE" );
    exit( 1 );
  }
}

Socket::Packet Socket::recv( void ) const
{
  const unsigned int BUF_SIZE = 2048;
  char buf[ BUF_SIZE ];
  
  struct sockaddr_in remote_addr;
  socklen_t addrlen = sizeof( remote_addr );

  ssize_t received_len = recvfrom( sock, buf, BUF_SIZE, 0, (sockaddr *)&remote_addr, &addrlen );

  if ( received_len < 0 ) {
    perror( "recvfrom" );
    exit( 1 );
  }

  if ( received_len > BUF_SIZE ) {
    fprintf( stderr, "Received oversize datagram (size %d) and limit is %d\n",
	     static_cast<int>( received_len ), BUF_SIZE );
    exit( 1 );
  }

  return Socket::Packet( Socket::Address( remote_addr ),
			 string( buf, received_len ) );
}
