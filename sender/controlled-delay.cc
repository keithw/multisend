#include <string>
#include <vector>
#include <poll.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <deque>

#include "socket.hh"
#include "rate-estimate.hh"
#include "history.hh"
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
 #ifdef EMULATE
  Socket::Address ethernet_address( "127.0.0.1", 9000 );
  ethernet_socket.bind( ethernet_address );
  ethernet_socket.bind_to_device( "lo" );
 #else 
  Socket::Address ethernet_address( "128.30.76.255", 9000 );
  ethernet_socket.bind( ethernet_address );
  ethernet_socket.bind_to_device( "eth0" );
 #endif

  /* Create and bind LTE socket */
  Socket lte_socket;

 #ifdef EMULATE
  lte_socket.bind( Socket::Address( "127.0.0.1", 9001 ) );
  lte_socket.bind_to_device( "lo" );
 #else 
  lte_socket.bind( Socket::Address( "10.100.1.1", 9001 ) );
  lte_socket.bind_to_device( "usb0" );
 #endif

  /* Figure out the NAT addresses of each of the three LTE sockets */
  Socket::Address target( get_nat_addr( lte_socket, ethernet_address, ethernet_socket ) );
  fprintf( stderr, "LTE = %s\n", target.str().c_str() );

  RateEstimate rate_estimator( 50.0, 1000 );
  
  const unsigned int PACKET_SIZE = 1400;
  unsigned int packets_sent = 0;
  unsigned int packets_received = 0;

  const double QUEUE_DURATION_TARGET = 1.0;
  const double STEERING_TIME = 0.05;

  int my_pid = (int) getpid();

  uint64_t next_transmission = Socket::timestamp();
  uint64_t last_transmission = next_transmission;

  History hist;

  const double minimum_rate = 5; /* packets per second */

  uint64_t first_microtick = -1;

  while ( 1 ) {
    fflush( NULL );
    
    uint64_t now = Socket::timestamp();

    /* schedule next packet transmission */

    /* Q: When will queue hit our target? */
    /* Step 1: Estimate queue size in ms */

    double queue_duration_estimate = (double) hist.num_outstanding() / rate_estimator.get_rate(); /* in seconds */

    /* Step 2: At what rate, will queue duration hit the target exactly STEERING_TIME seconds in the future? */

    /* We want to account for this much difference over the next STEERING_TIME seconds */
    double queue_duration_difference = QUEUE_DURATION_TARGET - queue_duration_estimate;

    double extra_packets_needed = queue_duration_difference * rate_estimator.get_rate() + STEERING_TIME * rate_estimator.get_rate();
    double extra_packet_rate = extra_packets_needed / STEERING_TIME; /* packets per second */

    if ( extra_packet_rate < minimum_rate ) {
      extra_packet_rate = minimum_rate;
    }

    uint64_t interpacket_delay = 1.e9 / extra_packet_rate;

    if ( extra_packet_rate > 0 ) {
      /* schedule next transmission */
      next_transmission = last_transmission + interpacket_delay;
    } else {
      next_transmission = uint64_t( -1 ); /* never */
    }

    //    printf( "Queue duration estimate: %f\n", queue_duration_estimate );

    if ( next_transmission <= now ) {
      Payload outgoing;
      outgoing.sequence_number = packets_sent++;
      outgoing.sent_timestamp = Socket::timestamp();
      outgoing.process_id = my_pid;
      ethernet_socket.send( Socket::Packet( target, outgoing.str( PACKET_SIZE ) ) );
      hist.packet_sent( outgoing );
      last_transmission = outgoing.sent_timestamp;
    }

    if ( extra_packet_rate > 0 ) {
      /* schedule next transmission */
      next_transmission = last_transmission + interpacket_delay;
    } else {
      next_transmission = uint64_t( -1 ); /* never */
    }

    /* wait for incoming packet OR expiry of timer */

    struct pollfd poll_fds[ 1 ];
    poll_fds[ 0 ].fd = lte_socket.get_sock();
    poll_fds[ 0 ].events = POLLIN;

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
      if ( first_microtick == uint64_t(-1) ) {
	first_microtick = contents->recv_timestamp / 1000;
      }
      if ( contents->process_id == my_pid ) {
	rate_estimator.add_packet( *contents );
	hist.packet_received( *contents );
	packets_received++;
	double loss_rate = (double) hist.num_lost() / (double) packets_sent;
	printf( "seq = %d delay = %f recvrate = %f sendrate = %f queueest = %f outstanding = %d Mbps = %f lost = %.5f%% arrivemicro = %ld\n",
		contents->sequence_number,
		(contents->recv_timestamp - contents->sent_timestamp) / 1.0e6,
		rate_estimator.get_rate(),
		extra_packet_rate,
		(double) hist.num_outstanding() / rate_estimator.get_rate(),
		hist.num_outstanding(),
		rate_estimator.get_rate() * PACKET_SIZE * 8.0 / 1.0e6,
		loss_rate * 100,
		contents->recv_timestamp / 1000 - first_microtick );
      }
    }
  }
}
