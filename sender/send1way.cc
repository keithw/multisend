#include <string>
#include <vector>
#include <poll.h>
#include <assert.h>

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

  /* Send and get timestamps */
  const uint64_t NUM_PACKETS = 10000;

  struct pollfd pollfds[ 3 ];
  for ( int i = 0; i < 3; i++ ) {
    pollfds[ i ].fd = lte_socket[ i ].get_sock();
    pollfds[ i ].events = POLLIN;
  }

  const int MAX_OUTSTANDING = 30;
  vector<int> num_outstanding( 3, 0 );

  vector<uint64_t> sent_times( NUM_PACKETS );

  uint64_t packets_sent = 0;

  while ( packets_sent < NUM_PACKETS ) {
    /* send packet if any receivers can be replenished */
    for ( int i = 0; i < 3; i++ ) {
      if ( num_outstanding[ i ] < MAX_OUTSTANDING ) {
	int packets_to_send = MAX_OUTSTANDING - num_outstanding[ i ];
	for ( int pnum = 0; pnum < packets_to_send; pnum++ ) {
	  char *seq_encoded = (char *)&packets_sent;
	  ethernet_socket.send( Socket::Packet( target[ i ], string( seq_encoded, sizeof( packets_sent ) ) ) );
	  sent_times[ packets_sent ] = Socket::timestamp();
	  packets_sent++;
	  num_outstanding[ i ]++;
	}
      }
    }

    /* now, poll */
    if ( poll( pollfds, 3, -1 ) <= 0 ) {
      perror( "poll" );
      exit( 1 );
    }
    
    for ( int i = 0; i < 3; i++ ) {
      if ( pollfds[ i ].revents & POLLERR ) {
	fprintf( stderr, "Error on LTE %d\n", i );
	exit( 1 );
      }

      if ( pollfds[ i ].revents & POLLIN ) {
	Socket::Packet rec = lte_socket[ i ].recv();
	assert ( rec.payload.size() == sizeof( packets_sent ) );
	uint64_t seq = *(uint64_t *)rec.payload.data();
	fprintf( stderr, "%d %ld %ld %ld\n", i, seq, sent_times[ seq ], rec.timestamp );
	num_outstanding[ i ]--;
      }
    }
  }
}
