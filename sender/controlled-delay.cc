#include <string>
#include <vector>
#include <poll.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

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

  DelayServo controlled_delay( ethernet_socket, target, lte_socket );

  while ( 1 ) {
    fflush( NULL );

    /* possibly send packet */
    controlled_delay.tick();
    
    /* wait for incoming packet OR expiry of timer */
    struct pollfd poll_fds[ 1 ];
    poll_fds[ 0 ].fd = controlled_delay.fd();
    poll_fds[ 0 ].events = POLLIN;

    struct timespec timeout;
    uint64_t next_transmission_delay = controlled_delay.wait_time_ns();
    timeout.tv_sec = next_transmission_delay / 1000000000;
    timeout.tv_nsec = next_transmission_delay % 1000000000;
    ppoll( poll_fds, 1, &timeout, NULL );

    if ( poll_fds[ 0 ].revents & POLLIN ) {
      controlled_delay.recv();
    }
  }
}
