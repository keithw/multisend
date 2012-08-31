#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "delay-servo.hh"

DelayServo::DelayServo( const std::string & s_name, const Socket & s_sender,
			const Socket::Address & s_target, const Socket & s_receiver )
  : _name( s_name ), 
    _sender( s_sender ),
    _target( s_target ),
    _receiver( s_receiver ),
    _rate_estimator( 50.0, 1000 ),
    _packets_sent( 0 ),
    _packets_received( 0 ),
    _unique_id( (int) getpid() ^ rand() ),
    _next_transmission( Socket::timestamp() ),
    _last_transmission( _next_transmission ),
    _hist()
{
}

int DelayServo::wait_time_ns( void ) const
{
  return _next_transmission - Socket::timestamp();
}

void DelayServo::recv( void )
{
  Socket::Packet incoming( _receiver.recv() );
  Payload *contents = (Payload *) incoming.payload.data();
  contents->recv_timestamp = incoming.timestamp;

  if ( contents->sender_id == _unique_id ) {
    _rate_estimator.add_packet( *contents );
    _hist.packet_received( *contents );
    _packets_received++;
    double loss_rate = (double) _hist.num_lost() / (double) _packets_sent;
    printf( "%s seq = %d delay = %f recvrate = %f queueest = %f outstanding = %d Mbps = %f lost = %.5f%% arrivemilli = %ld\n",
	    _name.c_str(),
	    contents->sequence_number,
	    (double) (contents->recv_timestamp - contents->sent_timestamp) / 1.0e9,
	    _rate_estimator.get_rate(),
	    (double) _hist.num_outstanding() / _rate_estimator.get_rate(),
	    _hist.num_outstanding(),
	    _rate_estimator.get_rate() * PACKET_SIZE * 8.0 / 1.0e6,
	    loss_rate * 100,
	    contents->recv_timestamp / 1000000 );
  }
}

void DelayServo::tick( void )
{
  uint64_t now = Socket::timestamp();

  /* schedule next packet transmission */

  /* Q: When will queue hit our target? */
  /* Step 1: Estimate queue size in ms */

  double queue_duration_estimate = (double) _hist.num_outstanding() / _rate_estimator.get_rate(); /* in seconds */

  /* Step 2: At what rate, will queue duration hit the target exactly STEERING_TIME seconds in the future? */

  /* We want to account for this much difference over the next STEERING_TIME seconds */
  double queue_duration_difference = QUEUE_DURATION_TARGET - queue_duration_estimate;

  double outgoing_packets_needed = queue_duration_difference * _rate_estimator.get_rate() + STEERING_TIME * _rate_estimator.get_rate();
  double outgoing_packet_rate = std::max( MINIMUM_RATE,
					  outgoing_packets_needed / STEERING_TIME ); /* packets per second */

  uint64_t interpacket_delay = 1.e9 / outgoing_packet_rate;

  assert( outgoing_packet_rate > 0 );

  _next_transmission = _last_transmission + interpacket_delay;

  if ( _next_transmission <= now ) {
    /* Send packet */
    Payload outgoing;
    outgoing.sequence_number = _packets_sent++;
    outgoing.sent_timestamp = Socket::timestamp();
    outgoing.sender_id = _unique_id;
    _sender.send( Socket::Packet( _target, outgoing.str( PACKET_SIZE ) ) );
    _hist.packet_sent( outgoing );
    _last_transmission = outgoing.sent_timestamp;
  }

  /* schedule next transmission */
  _next_transmission = _last_transmission + interpacket_delay;
}
