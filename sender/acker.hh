#ifndef ACKER_HH
#define ACKER_HH

#include "socket.hh"

class SaturateServo;

class Acker
{
private:
  const std::string _name;

  const Socket _listen;
  const Socket _send;
  Socket::Address _remote;
  const bool _server;
  const int _ack_id;

  SaturateServo *_saturatr;

  uint64_t _next_ping_time;

  static const int _ping_interval = 1000000000;

  int _foreign_id;

public:
  Acker( const char * s_name,
	 const Socket & s_listen,
	 const Socket & s_send,
	 const Socket::Address & s_remote,
	 const bool s_server,
	 const int s_ack_id );
  void recv( void );
  void tick( void );

  void set_remote( const Socket::Address & s_remote ) { _remote = s_remote; }

  void set_saturatr( SaturateServo * const s_saturatr ) { _saturatr = s_saturatr; }

  uint64_t wait_time( void ) const;

  Acker( const Acker & ) = delete;
  const Acker & operator=( const Acker & ) = delete;
};

#endif
