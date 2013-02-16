#ifndef TAP_DEVICE_HH
#define TAP_DEVICE_HH

#include<string.h>
#include<stdio.h>
#include<net/if.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/if_tun.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include <string>
#include <vector>
#include "macaddr.hh"

class TapSocket
{
private:
  int sock;

  const MACAddress _from_filter;
  const MACAddress _to_filter;

public:
  TapSocket( const std::string & s_interface,
		const std::string & s_from_filter,
		const std::string & s_to_filter );

  std::vector< std::string > recv_raw( void );
  void send_raw( const std::string & input );
  int fd( void ) const { return sock; }
  int tun_alloc( const char *dev, int flags) const;
  int setup_tap( const char* tap_name ) const;

};

#endif
