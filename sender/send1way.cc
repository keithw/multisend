#include "socket.hh"

int main( void )
{
  Socket lte1_socket, lte2_socket, lte3_socket, ethernet_socket;

  Socket::Address ethernet_address( "128.30.76.255", 9000 );
  Socket::Address lte1_local_address( "10.100.1.1", 9001 );
  Socket::Address lte2_local_address( "10.100.2.1", 9002 );
  Socket::Address lte3_local_address( "10.100.3.1", 9003 );

  ethernet_socket.bind( ethernet_address );
  lte1_socket.bind( lte1_local_address );
  lte2_socket.bind( lte2_local_address );
  lte3_socket.bind( lte3_local_address );

  ethernet_socket.bind_to_device( "eth0" );
  lte1_socket.bind_to_device( "usb0" );
  lte2_socket.bind_to_device( "usb1" );
  lte3_socket.bind_to_device( "usb2" );

  lte1_socket.send( Socket::Packet( ethernet_address, "Hello." ) );

  Socket::Packet received( ethernet_socket.recv() );

  printf( "From %s got \"%s\".\n", received.addr.str().c_str(), received.payload.c_str() );

  sleep( 2 );

  ethernet_socket.send( Socket::Packet( received.addr, "Goodbye." ) );

  Socket::Packet received2( lte1_socket.recv() );

  printf( "From %s got \"%s\".\n", received2.addr.str().c_str(), received2.payload.c_str() );

  return 0;
}
