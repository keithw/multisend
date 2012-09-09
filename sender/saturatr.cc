#include <string>
#include <vector>
#include <poll.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

#include "acker.hh"
#include "saturateservo.hh"

using namespace std;

int main( int argc, char *argv[] )
{
  if ( argc != 1 && argc != 7 ) {
    fprintf( stderr, "Usage: %s [RELIABLE_IP RELIABLE_DEV TEST_IP TEST_DEV SERVER_IP ID]\n",
	     argv[ 0 ]);
    exit( 1 );
  }

  Socket data_socket, feedback_socket;
  bool server;

  int sender_id = getpid();

  Socket::Address remote_data_address( UNKNOWN ), remote_feedback_address( UNKNOWN );

  if ( argc == 1 ) { /* server */
    server = true;
    data_socket.bind( Socket::Address( "0.0.0.0", 9001 ) );
    feedback_socket.bind( Socket::Address( "0.0.0.0", 9002 ) );
  } else { /* client */
    server = false;
    
    const char *reliable_ip = argv[ 1 ];
    const char *reliable_dev = argv[ 2 ];

    const char *test_ip = argv[ 3 ];
    const char *test_dev = argv[ 4 ];

    const char *server_ip = argv[ 5 ];

    sender_id = atoi( argv[ 6 ] );

    data_socket.bind( Socket::Address( test_ip, 9003 ) );
    data_socket.bind_to_device( test_dev );
    remote_data_address = Socket::Address( server_ip, 9001 );

    feedback_socket.bind( Socket::Address( reliable_ip, 9004 ) );
    feedback_socket.bind_to_device( reliable_dev );
    remote_feedback_address = Socket::Address( server_ip, 9002 );
  }

  FILE* log_file;
  uint64_t ts=Socket::timestamp();
  char log_file_name[20];
  sprintf(log_file_name,"%s-%d-%d",server ? "server" : "client",(int)(ts/1e9),sender_id);
  log_file=fopen(log_file_name,"w");

  SaturateServo saturatr( "OUTGOING", log_file, feedback_socket, data_socket, remote_data_address, server, sender_id );
  Acker acker( "INCOMING", log_file, data_socket, feedback_socket, remote_feedback_address, server, sender_id );

  saturatr.set_acker( &acker );
  acker.set_saturatr( &saturatr );

  while ( 1 ) {
    fflush( NULL );

    /* possibly send packet */
    saturatr.tick();
    acker.tick();
    
    /* wait for incoming packet OR expiry of timer */
    struct pollfd poll_fds[ 2 ];
    poll_fds[ 0 ].fd = data_socket.get_sock();
    poll_fds[ 0 ].events = POLLIN;
    poll_fds[ 1 ].fd = feedback_socket.get_sock();
    poll_fds[ 1 ].events = POLLIN;

    struct timespec timeout;
    uint64_t next_transmission_delay = std::min( saturatr.wait_time(), acker.wait_time() );

    if ( next_transmission_delay == 0 ) {
      fprintf( stderr, "ZERO %ld %ld\n", saturatr.wait_time(), acker.wait_time() );
    }

    timeout.tv_sec = next_transmission_delay / 1000000000;
    timeout.tv_nsec = next_transmission_delay % 1000000000;
    ppoll( poll_fds, 2, &timeout, NULL );

    if ( poll_fds[ 0 ].revents & POLLIN ) {
      acker.recv();
    }

    if ( poll_fds[ 1 ].revents & POLLIN ) {
      saturatr.recv();
    }
  }
}
