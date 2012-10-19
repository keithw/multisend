#ifndef DELAY_SERVO_HH
#define DELAY_SERVO_HH

#include <string>

#include "socket.hh"
#include "rate-estimate.hh"
#include "history.hh"

class DelayServo {
private:

  const std::string _name;

  const Socket & _sender;
  const Socket::Address & _target;
  const Socket & _receiver;

  RateEstimate _rate_estimator;

  unsigned int _packets_sent, _packets_received;

  static const unsigned int PACKET_SIZE = 1400; /* bytes */
  static constexpr double QUEUE_DURATION_TARGET = 1.5; /* seconds */
  static constexpr double STEERING_TIME = 0.2; /* seconds */
  static constexpr double MINIMUM_RATE = 10.0; /* packets per second */

  int _unique_id;

  uint64_t _next_transmission, _last_transmission;

  History _hist;

public:

  DelayServo( const std::string & s_name, const Socket & s_sender,
	      const Socket::Address & s_target, const Socket & s_receiver );

  void tick( void );
  uint64_t recv( void );

  int wait_time_ns( void ) const;
  int fd( void ) const { return _receiver.get_sock(); }
  double loss_rate( void ) const;
};

#endif
