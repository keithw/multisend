#include <assert.h>

#include "saturateservo.hh"
#include "socket.hh"
#include "payload.hh"
#include "acker.hh"

SaturateServo::SaturateServo( const char * s_name,
			      const Socket & s_listen,
			      const Socket & s_send,
			      const Socket::Address & s_remote,
			      const bool s_server,
			      const int s_send_id )
  : _name( s_name ),
    _listen( s_listen ),
    _send( s_send ),
    _remote( s_remote ),
    _server( s_server ),
    _send_id( s_send_id ),
    _acker( NULL ),
    _next_transmission_time( Socket::timestamp() ),
    _foreign_id( -1 ),
    _packets_sent( 0 ),
    _max_ack_id( -1 ),
    _window( LOWER_WINDOW )
{}

void SaturateServo::recv( void )
{
  /* get the ack packet */
  Socket::Packet incoming( _listen.recv() );
  SatPayload *contents = (SatPayload *) incoming.payload.data();
  contents->recv_timestamp = incoming.timestamp;

  if ( contents->sequence_number != -1 ) {
    /* not an ack */
    printf( "MARTIAN!\n" );
    return;
  }

  /* possibly roam */
  if ( _server ) {
    if ( _acker ) {
      if ( contents->sender_id > _foreign_id ) {
	_foreign_id = contents->sender_id;
	_acker->set_remote( incoming.addr );
      }
    }
  }

  /* process the ack */
  if ( contents->sender_id != _send_id ) {
    /* not from us */
    return;
  } else {
    if ( contents->ack_number > _max_ack_id ) {
      _max_ack_id = contents->ack_number;
    }

    /*    printf( "%s pid=%d ACK RECEIVED senderid=%d seq=%d, send_time=%ld, recv_time=%ld\n",
	  _name.c_str(), getpid(), contents->sender_id, contents->sequence_number, contents->sent_timestamp, contents->recv_timestamp ); */

    int64_t rtt_ns = contents->recv_timestamp - contents->sent_timestamp;
    double rtt = rtt_ns / 1.e9;

    printf( "%d rtt: %.4f %d => ", contents->ack_number, (double) rtt, _window );

    /* increase-decrease rules */

    if ( (rtt < LOWER_RTT) && (_window < UPPER_WINDOW) ) {
      _window++;
    }

    if ( (rtt > UPPER_RTT) && (_window > LOWER_WINDOW + 10) ) {
      _window -= 20;
    }

    printf( "%d\n", _window );
  }
}

uint64_t SaturateServo::wait_time( void ) const
{
  int num_outstanding = _packets_sent - _max_ack_id - 1;

  if ( _remote == UNKNOWN ) {
    return 1000000000;
  }

  if ( num_outstanding < _window ) {
    return 0;
  } else {
    int diff = _next_transmission_time - Socket::timestamp();
    if ( diff < 0 ) {
      diff = 0;
    }
    return diff;
  }
}

void SaturateServo::tick( void )
{
  if ( _remote == UNKNOWN ) {
    return;
  }

  int num_outstanding = _packets_sent - _max_ack_id - 1;

  if ( num_outstanding < _window ) {
    /* send more packets */
    int amount_to_send = _window - num_outstanding;
    for ( int i = 0; i < amount_to_send; i++ ) {
      SatPayload outgoing;
      outgoing.sequence_number = _packets_sent;
      outgoing.ack_number = -1;
      outgoing.sent_timestamp = Socket::timestamp();
      outgoing.recv_timestamp = 0;
      outgoing.sender_id = _send_id;

      _send.send( Socket::Packet( _remote, outgoing.str( 1400 ) ) );

      /*
      printf( "%s pid=%d DATA SENT %d senderid=%d seq=%d, send_time=%ld, recv_time=%ld\n",
      _name.c_str(), getpid(), amount_to_send, outgoing.sender_id, outgoing.sequence_number, outgoing.sent_timestamp, outgoing.recv_timestamp ); */

      _packets_sent++;
    }

    _next_transmission_time = Socket::timestamp() + _transmission_interval;
  }

  if ( _next_transmission_time < Socket::timestamp() ) {
    SatPayload outgoing;
    outgoing.sequence_number = _packets_sent;
    outgoing.ack_number = -1;
    outgoing.sent_timestamp = Socket::timestamp();
    outgoing.recv_timestamp = 0;
    outgoing.sender_id = _send_id;

    _send.send( Socket::Packet( _remote, outgoing.str( 1400 ) ) );

    /*
    printf( "%s pid=%d DATA SENT senderid=%d seq=%d, send_time=%ld, recv_time=%ld\n",
    _name.c_str(), getpid(), outgoing.sender_id, outgoing.sequence_number, outgoing.sent_timestamp, outgoing.recv_timestamp ); */

    _packets_sent++;

    _next_transmission_time = Socket::timestamp() + _transmission_interval;
  }
}
