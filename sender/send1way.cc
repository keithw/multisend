#include <string>
#include <vector>

#include "socket.hh"

using namespace std;

Socket::Address get_nat_addr( const Socket & sender, const Socket::Address & dest,
			      const Socket & receiver )
{
  char buf[ 10 ];
  for ( int i = 0; i < 10; i++ ) {
    buf[ i ] = rand() % 256;
  }
  
  string to_send( buf, 10 );

  sender.send( Socket::Packet( dest, to_send ) );
  Socket::Packet received( receiver.recv() );

  if ( received.payload != to_send ) {
    fprintf( stderr, "Bad packet received while getting NAT addresses.\n" );
    exit( 1 );
  }

  return received.addr;
}

int main( void )
{
  /* Create and bind Ethernet socket */
  Socket ethernet_socket;
  Socket::Address ethernet_address( "128.30.76.255", 9000 );
  ethernet_socket.bind( ethernet_address );
  ethernet_socket.bind_to_device( "eth0" );

  /* Create and bind three LTE sockets */
  vector<Socket> lte_socket;

  for ( int i = 0; i < 3; i++ ) {
    lte_socket.push_back( Socket() );
  }

  lte_socket[ 0 ].bind( Socket::Address( "10.100.1.1", 9001 ) );
  lte_socket[ 0 ].bind_to_device( "usb0" );
  lte_socket[ 1 ].bind( Socket::Address( "10.100.2.1", 9002 ) );
  lte_socket[ 1 ].bind_to_device( "usb1" );
  lte_socket[ 2 ].bind( Socket::Address( "10.100.3.1", 9003 ) );
  lte_socket[ 2 ].bind_to_device( "usb2" );

  /* Figure out the NAT addresses of each of the three LTE sockets */
  vector<Socket::Address> target;
  
  for ( int i = 0; i < 3; i++ ) {
    target.push_back( get_nat_addr( lte_socket[ i ], ethernet_address, ethernet_socket ) );
    printf( "LTE %d = %s\n", i, target.back().str().c_str() );
  }

  /* Send and get timestamp */
  ethernet_socket.send( Socket::Packet( target[ 1 ], "Hello." ) );
  uint64_t ts_sent = Socket::timestamp();

  Socket::Packet rec = lte_socket[ 1 ].recv();

  uint64_t latency = rec.timestamp - ts_sent;

  printf( "Latency: %.10f ms\n", latency / 1000000.0 );

  return 0;
}
