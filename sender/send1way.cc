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
    fprintf( stderr, "LTE %d = %s\n", i, target.back().str().c_str() );
  }

  /* Make dummy address */
  Socket::Address dummy_address( "220.181.111.147", 9004 );

  /* Send and get timestamps */
  const uint64_t NUM_PACKETS = 1400000;

  struct pollfd pollfds[ 3 ];
  for ( int i = 0; i < 3; i++ ) {
    pollfds[ i ].fd = lte_socket[ i ].get_sock();
    pollfds[ i ].events = POLLIN;
  }

  const int MAX_OUTSTANDING = 1000;
  vector<int> num_outstanding( 3 );

  struct packetstat {
    uint64_t sent_time;
    int dest;
    bool received;
  };

  vector<struct packetstat> packetstats( NUM_PACKETS );
  uint64_t next_unacked = 0;

  uint64_t packets_sent = 0;

  uint64_t base_time = Socket::timestamp();

  const uint64_t TIMEOUT = 500 * 1000 * 1000; /* 1 second */

  uint64_t last_time = base_time;

  vector<int> last_sec_count( 3 );

  int numextra = 0;

  while ( packets_sent < NUM_PACKETS ) {
    uint64_t the_time = Socket::timestamp();

    if ( (the_time / 100000000) != (last_time / 100000000) ) {
      if ( (the_time / 20000000000) != (last_time / 20000000000) ) {
	numextra = (numextra + 1) % 3;
	fprintf( stderr, "%ld FLIP %d\n", (the_time - base_time) / 10000000, numextra );
      }

      last_time = the_time;
      printf( "%ld %d %d %d\n", (the_time - base_time) / 100000000, last_sec_count[ 0 ], last_sec_count[ 1 ], last_sec_count[ 2 ] );
      fflush( NULL );
      for ( int i = 0; i < 3; i++ ) {
	last_sec_count[ i ] = 0;
	char *seq_encoded = (char *)&packets_sent;
	ethernet_socket.send( Socket::Packet( target[ i ], string( seq_encoded, 51 ) ) );
	packetstats[ packets_sent ].sent_time = Socket::timestamp();
	packetstats[ packets_sent ].dest = i;
	packets_sent++;
	num_outstanding[ i ]++;
      }
    }

    /* update next_unacked */
    while ( packetstats[ next_unacked ].received ) {
      next_unacked++;
      /* possible timeout */
      if ( !packetstats[ next_unacked ].received ) {
	if ( the_time - packetstats[ next_unacked ].sent_time > TIMEOUT ) {
	  fprintf( stderr, "Timeout on %ld (sent to %d)\n", next_unacked, packetstats[ next_unacked ].dest );
	  packetstats[ next_unacked ].received = true;
	  num_outstanding[ packetstats[ next_unacked ].dest ]--;
	}
      }
    }

    /* send packet if any receivers can be replenished */
    for ( int i = 0; i < 1 + numextra; i++ ) {
      if ( num_outstanding[ i ] < MAX_OUTSTANDING ) {
	int packets_to_send = MAX_OUTSTANDING - num_outstanding[ i ];
	for ( int pnum = 0; pnum < packets_to_send; pnum++ ) {
	  char *seq_encoded = (char *)&packets_sent;
	  ethernet_socket.send( Socket::Packet( target[ i ], string( seq_encoded, 51 ) ) );
	  packetstats[ packets_sent ].sent_time = Socket::timestamp();
	  packetstats[ packets_sent ].dest = i;
	  packets_sent++;
	  num_outstanding[ i ]++;

	  if ( i == 0 ) {
	    if ( numextra < 2 ) {
	      ethernet_socket.send( Socket::Packet( dummy_address, string( seq_encoded, 51 ) ) );
	    }
	    if ( numextra < 1 ) {
	      ethernet_socket.send( Socket::Packet( dummy_address, string( seq_encoded, 51 ) ) );
	    }
	  }
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
	//	assert ( rec.payload.size() == sizeof( packets_sent ) );
	uint64_t seq = *(uint64_t *)rec.payload.data();
	last_sec_count[ i ]++;
	assert( packetstats[ seq ].dest == i );
	if ( !packetstats[ seq ].received ) {
	  packetstats[ seq ].received = true;
	  num_outstanding[ i ]--;
	} else {
	  fprintf( stderr, "Packet %ld arrived after timeout!\n", seq );
	}

	/*
	double sent_ts = double(packetstats[ seq ].sent_time - base_time) / 1000000.0;
	double recv_ts = double(rec.timestamp - base_time) / 1000000.0;
	*/
	//	printf( "%d %ld %.8f %.8f delay=%.8f %d oldest-queued=%ld\n", i, seq, sent_ts, recv_ts, recv_ts - sent_ts, num_outstanding[ i ], packets_sent - next_unacked );
      }
    }
  }
}
