#include <string>
#include <vector>
#include <poll.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

#include "socket.hh"
#include "rate-estimate.hh"

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

  RateEstimate rate_estimator( 30.0, 500 );
  
  const unsigned int PACKET_SIZE = 600;
  unsigned int packets_outstanding = 0;
  unsigned int packets_sent = 0;

  const double QUEUE_DURATION_TARGET = 2.0;
  const double STEERING_TIME = 5.0;

  int my_pid = (int) getpid();

  uint64_t next_transmission = Socket::timestamp();

  while ( 1 ) {
    fflush( NULL );
    
    uint64_t now = Socket::timestamp();

    if ( next_transmission <= now ) {
      Payload outgoing;
      outgoing.sequence_number = packets_sent++;
      outgoing.sent_timestamp = Socket::timestamp();
      outgoing.process_id = my_pid;
      printf( "SENDING...\n" );
      ethernet_socket.send( Socket::Packet( target, outgoing.str( PACKET_SIZE ) ) );
      packets_outstanding++;

      /* schedule next packet transmission */

      /* Q: When will queue hit our target? */
      /* Step 1: Estimate queue size in ms */

      double queue_duration_estimate = (double) packets_outstanding / rate_estimator.get_rate(); /* in seconds */

      /* Step 2: At what rate, will queue duration hit the target exactly STEERING_TIME seconds in the future? */

      /* We want to account for this much difference over the next STEERING_TIME seconds */
      double queue_duration_difference = QUEUE_DURATION_TARGET - queue_duration_estimate;

      double extra_packets_needed = queue_duration_difference * rate_estimator.get_rate() + STEERING_TIME * rate_estimator.get_rate();
      double extra_packet_rate = extra_packets_needed / STEERING_TIME; /* packets per second */

      if ( extra_packet_rate > 0 ) {
	/* schedule next transmission */
	next_transmission = now + (uint64_t) (1.e9 / extra_packet_rate);
      } else {
	next_transmission = uint64_t( -1 ); /* never */
      }

      printf( "Queue duration estimate: %f\n", queue_duration_estimate );
    }

    /* wait for incoming packet OR expiry of timer */

    struct pollfd poll_fds[ 1 ];
    poll_fds[ 0 ].fd = lte_socket.get_sock();
    poll_fds[ 0 ].events = POLLIN;

    assert( next_transmission >= now );

    if ( next_transmission == uint64_t( -1 ) ) {
      ppoll( poll_fds, 1, NULL, NULL );
    } else {
      uint64_t next_transmission_delay = next_transmission - now;
      struct timespec timeout;
      timeout.tv_sec = next_transmission_delay / 1000000000;
      timeout.tv_nsec = next_transmission_delay % 1000000000;
      ppoll( poll_fds, 1, &timeout, NULL );
    }

    if ( poll_fds[ 0 ].revents & POLLIN ) {
      Socket::Packet incoming( lte_socket.recv() );
      Payload *contents = (Payload *) incoming.payload.data();
      contents->recv_timestamp = incoming.timestamp;
      if ( contents->process_id == my_pid ) {
	rate_estimator.add_packet( *contents );
	printf( "Rate estimate: %f packets per second [%d sent]\n", rate_estimator.get_rate(), packets_sent );
	printf( "Delay: %f ms\n", (contents->recv_timestamp - contents->sent_timestamp) / 1.0e6 );
	packets_outstanding--;
      }
    }
  }
}
