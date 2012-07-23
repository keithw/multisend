#include <string>
#include <vector>
#include <poll.h>
#include <assert.h>

#include "socket.hh"
#include "hist.hh"

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

double hread( uint64_t in )
{
  return (double) in / 1.e9;
}

int main( void )
{
  /* Create and bind Ethernet socket */
  Socket ethernet_socket;
  Socket::Address ethernet_address( "128.30.76.255", 9000 );
  ethernet_socket.bind( ethernet_address );
  ethernet_socket.bind_to_device( "eth0" );

  /* Create and bind LTE socket */
  Socket lte_socket;

  lte_socket.bind( Socket::Address( "10.100.1.1", 9001 ) );
  lte_socket.bind_to_device( "usb0" );

  /* Figure out the NAT addresses of each of the three LTE sockets */
  Socket::Address target( get_nat_addr( lte_socket, ethernet_address, ethernet_socket ) );
  fprintf( stderr, "LTE = %s\n", target.str().c_str() );

  /* Send packets and timestamps */
  const int MAX_PACKETS_EN_ROUTE = 4800, PACKET_SIZE = 400;
  int packets_en_route = 0;
  int packets_sent = 0, packets_received = 0;

  const int PACKETS_TO_SEND = 100000;

  struct pdata {
    uint64_t seq, ts;
    int pid;
  };

  assert( sizeof( struct pdata ) <= (size_t) PACKET_SIZE );

  pid_t mypid = getpid();

  uint64_t last_ts = 0;

  Histogram interarrival_hist( 1000 * 100 ); /* 100 us */

  while ( packets_received < PACKETS_TO_SEND ) {
    while ( packets_en_route < MAX_PACKETS_EN_ROUTE ) {
      struct pdata outgoing;
      outgoing.seq = packets_sent++;
      outgoing.ts = Socket::timestamp();
      outgoing.pid = mypid;
      Socket::Packet pack( target, string( (char *) &outgoing, PACKET_SIZE ) );
      ethernet_socket.send( pack );
      packets_en_route++;
    }

    /* Receive packet */
    Socket::Packet pack( lte_socket.recv() );
    struct pdata *contents = (struct pdata *) pack.payload.data();
    if ( contents->pid == mypid ) {
      //      uint64_t e2e_delay = pack.timestamp - contents->ts;
      uint64_t inter_delay = 0;

      if ( last_ts > 0 ) {
	inter_delay = pack.timestamp - last_ts;
	interarrival_hist.record( inter_delay );
      }

      if ( last_ts / 1000000000 != pack.timestamp / 1000000000 ) {
	fprintf( stderr, "Status: %d packets sent, %d received (%.2f%%)\n", packets_sent,
		 packets_received,
		 (double) packets_received * 100.0 / (double) packets_sent );	
      }

      last_ts = pack.timestamp;

      packets_en_route--;
      packets_received++;
    }
  }

  interarrival_hist.print();
}
