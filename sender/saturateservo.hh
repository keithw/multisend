#ifndef SATURATESERVO_HH
#define SATURATESERVO_HH

#include "socket.hh"

class Acker;

class SaturateServo
{
private:
  const std::string _name;

  const Socket _listen;
  const Socket _send;
  Socket::Address _remote;

  const bool _server;
  const int _send_id;

  Acker *_acker;

  uint64_t _next_transmission_time;

  static const int _transmission_interval = 1000 * 1000 * 1000;

  int _foreign_id;

  int _packets_sent, _max_ack_id;

  int _window;

  static const int LOWER_WINDOW = 20;
  static const int UPPER_WINDOW = 1500;

  static constexpr double LOWER_RTT = 0.75;
  static constexpr double UPPER_RTT = 3.0;

public:
  SaturateServo( const char * s_name,
		 const Socket & s_listen,
		 const Socket & s_send,
		 const Socket::Address & s_remote,
		 const bool s_server,
		 const int s_send_id );

  void recv( void );

  uint64_t wait_time( void ) const;

  void tick( void );

  void set_acker( Acker * const s_acker ) { _acker = s_acker; }

  void set_remote( const Socket::Address & s_remote ) { _remote = s_remote; }

  SaturateServo( const SaturateServo & ) = delete;
  const SaturateServo & operator=( const SaturateServo & ) = delete;
};

#endif
