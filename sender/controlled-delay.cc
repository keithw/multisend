#include <string>
#include <vector>
#include <poll.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "delay-servo.hh"

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
  Socket::Packet received( UNKNOWN, "" );
  
  for ( int tries = 0; tries < 20; tries++ ) {
    try {
      received = receiver.recv();
      break;
    } catch ( int x ) {
      if ( x == EAGAIN || x == EWOULDBLOCK ) {
	sleep( 1 );
      } else {
	exit( 1 );
      }
    }
  }

  if ( received.payload != to_send ) {
    fprintf( stderr, "Bad packet received while getting NAT addresses.\n" );
    for (int i = 0; i < 10; i++) {
      if (received.payload.at(i) != buf[i]) {
	fprintf( stderr, "First incorrect character at position %d (got %c, should be %c).\n", i, received.payload.at(i), buf[i] );
	exit( 1 );
      }
    }
    assert(false); // we should never get here
  }

  return received.addr;
}

double hread( uint64_t in )
{
  return (double) in / 1.e9;
}

int main( int argc, char** argv)
{
  if (argc < 6) {
    fprintf(stderr, "usage: controlled-delay eth0-ip eth0-port lte-ip lte-port lte-device\n");
    exit(1);
  }
  char* eth0_ip = argv[1];
  int eth0_port = atoi(argv[2]);
  char* lte_ip = argv[3];
  int lte_port = atoi(argv[4]);
  char* lte_device = argv[5];

  /* Create and bind Ethernet socket */
  Socket ethernet_socket;
 #ifdef EMULATE
  Socket::Address ethernet_address( "127.0.0.1", 9000 );
  ethernet_socket.bind( ethernet_address );
  ethernet_socket.bind_to_device( "lo" );
 #else 
  Socket::Address ethernet_address( eth0_ip, eth0_port );
  ethernet_socket.bind( ethernet_address );
  ethernet_socket.bind_to_device( "eth0" );
 #endif

  /* Create and bind LTE socket */
  Socket lte_socket;

 #ifdef EMULATE
  lte_socket.bind( Socket::Address( "127.0.0.1", 9001 ) );
  lte_socket.bind_to_device( "lo" );
 #else 
  lte_socket.bind( Socket::Address( lte_ip , lte_port ) );
  lte_socket.bind_to_device( lte_device );
 #endif

  /* Figure out the NAT addresses of each of the three LTE sockets */
  Socket::Address target( get_nat_addr( lte_socket, ethernet_address, ethernet_socket ) );

  DelayServo downlink( "DOWN", ethernet_socket, target, lte_socket );
  DelayServo uplink( "UP  ", lte_socket, ethernet_address, ethernet_socket );

  const int duration = 40;
  uint64_t num_bits_down[duration];
  uint64_t num_bits_up[duration];
  double downlink_loss_rate[duration];
  double uplink_loss_rate[duration];
  for (int i = 0; i < duration; i++) {
    num_bits_down[i] = 0;
    num_bits_up[i] = 0;
    downlink_loss_rate[i] = 0.0;
    uplink_loss_rate[i] = 0.0;
  }

  uint64_t start_time = Socket::timestamp(), running_time;
  while ( 1 ) {
    running_time = Socket::timestamp()-start_time;
    if (running_time >= (uint64_t)1000000000*duration) {
      break;
    }

    fflush( NULL );

    /* possibly send packet */
    downlink.tick();
    uplink.tick();
    
    /* wait for incoming packet OR expiry of timer */
    struct pollfd poll_fds[ 2 ];
    poll_fds[ 0 ].fd = downlink.fd();
    poll_fds[ 0 ].events = POLLIN;
    poll_fds[ 1 ].fd = uplink.fd();
    poll_fds[ 1 ].events = POLLIN;

    struct timespec timeout;
    uint64_t next_transmission_delay = std::min( uplink.wait_time_ns(), downlink.wait_time_ns() );
    timeout.tv_sec = next_transmission_delay / 1000000000;
    timeout.tv_nsec = next_transmission_delay % 1000000000;
    ppoll( poll_fds, 2, &timeout, NULL );

    if ( poll_fds[ 0 ].revents & POLLIN ) {
      num_bits_down[ running_time / 1000000000 ] += downlink.recv();
    }

    if ( poll_fds[ 1 ].revents & POLLIN ) {
      num_bits_up[ running_time / 1000000000 ] += uplink.recv();
    }

    downlink_loss_rate[ running_time / 1000000000 ] = downlink.loss_rate();
    uplink_loss_rate[ running_time / 1000000000 ] = uplink.loss_rate();
  }

  for (int i = 0; i < duration; i++) {
    printf("%s %ld %ld %ld %.5f %.5f\n", lte_device, start_time / 1000000000 + i, num_bits_down[i], num_bits_up[i], downlink_loss_rate[i], uplink_loss_rate[i]);
  }
}
