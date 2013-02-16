#include "tapsocket.hh"
#include <assert.h>

using namespace std;

int TapSocket::setup_tap( const char* tap_name ) const
{
  int tap_fd = tun_alloc( tap_name, IFF_TAP | IFF_NO_PI );
  if ( tap_fd < 0 ) {
    perror( "Allocating interface" );
    exit( 1 );
  }
  return tap_fd;
}

int TapSocket::tun_alloc(const char *dev, int flags) const
{
  struct ifreq ifr;
  int fd;
  int err;
  std::string clone_dev = "/dev/net/tun";
              
  /* open the clone device */
  if( (fd = open(clone_dev.c_str(), O_RDWR)) < 0 ) {
    return fd;
  }

  /* zero out struct ifr, of type "struct ifreq" */
  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, optionally IFF_NO_PI */
                    
  /* if a device name is supplied, use it */
  assert( dev != nullptr );
  strncpy(ifr.ifr_name, dev, IFNAMSIZ);
                    
  /* try to create the device */
  if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
    close(fd);
    return err;
  }
                        
  /* return file descriptor to tap device */
  return fd;
}


TapSocket::TapSocket( const std::string & s_interface,
			    const std::string & s_from_filter,
			    const std::string & s_to_filter )
  : sock( setup_tap( s_interface.c_str() ) ),
    _from_filter( MACAddress::parse_human( s_from_filter ) ),
    _to_filter( MACAddress::parse_human( s_to_filter ) )
{}

vector< string > TapSocket::recv_raw( void )
{
  vector< string > ret;

  char buffer[1600];
  int nread = read( sock, (void*) buffer, sizeof(buffer) );
  string packet( buffer, nread );

  if(nread < 0) {
    perror("Reading from interface");
    close(sock);
    exit(1);
  }

  assert( packet.size() > 12 );

  const MACAddress destination_address( packet.substr( 0, 6 ) );
  const MACAddress source_address( packet.substr( 6, 6 ) );

  if (  _to_filter.matches( destination_address )
	&& _from_filter.matches( source_address ) ) {
    ret.push_back( packet );
  }

  return ret;
}

void TapSocket::send_raw( const std::string & input )
{
  /* write into tap device */
  ssize_t bytes_sent = write( sock, input.c_str(), input.size() );
  if ( bytes_sent < 0 ) {
    perror( "send" );
    exit( 1 );
  }
}
