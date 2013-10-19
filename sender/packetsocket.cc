#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>

#include "packetsocket.hh"

using namespace std;

int PacketSocket::get_index( const std::string & name ) const
{
  /* Find index number from name */
  struct ifreq ifr;

  strncpy( ifr.ifr_name, name.c_str(), IFNAMSIZ );
  if ( ioctl( sock, SIOCGIFINDEX, &ifr ) < 0 ) {
    perror( "ioctl" );
    exit( 1 );
  }

  fprintf( stderr, "Interface %s has index %d\n", name.c_str(), ifr.ifr_ifindex );
  
  return ifr.ifr_ifindex;
}

PacketSocket::PacketSocket( const std::string & s_interface )
  : sock( socket( AF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) ) )
{
  /* create packet socket */

  if ( sock < 0 ) {
    perror( "socket" );
    exit( 1 );
  }

  /* Get interface index */
  int index = get_index( s_interface );

  /* Bind packet socket to interface */
  struct sockaddr_ll sll;
  sll.sll_family = AF_PACKET;
  sll.sll_protocol = htons( ETH_P_ALL );
  sll.sll_ifindex = index;

  if ( bind( sock, (struct sockaddr *)&sll, sizeof( sll ) ) < 0 ) {
    perror( "bind" );
    exit( 1 );
  }

  /* Set to promiscuous mode */
  struct packet_mreq pmr;
  pmr.mr_ifindex = index;
  pmr.mr_type = PACKET_MR_PROMISC;

  if ( setsockopt( sock, SOL_SOCKET, PACKET_ADD_MEMBERSHIP, &pmr, sizeof( pmr ) ) < 0 ) {
    perror( "setsockopt promisc" );
    exit( 1 );
  }
}

vector< string > PacketSocket::recv_raw( void )
{
  vector< string > ret;

  const int BUFFER_SIZE = 2048;

  char buf[ BUFFER_SIZE ];

  sockaddr_ll source_address;
  socklen_t source_address_len = sizeof( source_address );

  ssize_t bytes_read = recvfrom( sock, buf, BUFFER_SIZE, MSG_TRUNC,
				 reinterpret_cast<sockaddr *>( &source_address ),
				 &source_address_len );
  if ( bytes_read < 0 ) {
    perror( "recvfrom" );
    exit( 1 );
  } else if ( bytes_read > BUFFER_SIZE ) {
    fprintf( stderr, "Received size (%ld) too long (> %d)!\n",
	     bytes_read, BUFFER_SIZE );
    exit( 1 );
  }

  if ( source_address_len != sizeof( source_address ) ) {
    perror( "recvfrom (unexpected address length" );
    exit( 1 );
  }

  if ( source_address.sll_pkttype != PACKET_OUTGOING ) {
    ret.emplace_back( buf, bytes_read );
  }

  return ret;
}

void PacketSocket::send_raw( const std::string & input )
{
  ssize_t bytes_sent = send( sock, input.data(), input.size(), 0 );
  if ( bytes_sent < 0 ) {
    perror( "send" );
    exit( 1 );
  }
}


