#include <assert.h>

#include "acker.hh"
#include "payload.hh"
#include "saturateservo.hh"

Acker::Acker( const char *s_name, const Socket & s_listen, const Socket & s_send, const Socket::Address & s_remote, const bool s_server, const int s_ack_id )
  : _name( s_name ), 
    _listen( s_listen ),
    _send( s_send ),
    _remote( s_remote ),
    _server( s_server ),
    _ack_id( s_ack_id ),
    _saturatr( NULL ),
    _next_ping_time( Socket::timestamp() ),
    _foreign_id( -1 )
{}

void Acker::recv( void )
{
  /* get the data packet */
  Socket::Packet incoming( _listen.recv() );
  SatPayload *contents = (SatPayload *) incoming.payload.data();
  contents->recv_timestamp = incoming.timestamp;

  int64_t oneway_ns = contents->recv_timestamp - contents->sent_timestamp;
  double oneway = oneway_ns / 1.e9;
  printf( "%d 1delay: %.4f\n", contents->sequence_number, oneway );

  if ( _server ) {
    if ( _saturatr ) {
      if ( contents->sender_id > _foreign_id ) {
	_foreign_id = contents->sender_id;
	_saturatr->set_remote( incoming.addr );
      }
    }

    if ( _remote == UNKNOWN ) {
      return;
    }
  }

  assert( !(_remote == UNKNOWN) );

  Socket::Address fb_destination( _remote );

  /* send ack */
  SatPayload outgoing( *contents );
  outgoing.sequence_number = -1;
  outgoing.ack_number = contents->sequence_number;
  _send.send( Socket::Packet( _remote, outgoing.str( sizeof( SatPayload ) ) ) );

  /*  printf( "%s pid=%d DATA RECEIVED / ACK SENT senderid=%d seq=%d, send_time=%ld, recv_time=%ld\n",
      _name.c_str(), getpid(), contents->sender_id, contents->sequence_number, contents->sent_timestamp, contents->recv_timestamp ); */
}

void Acker::tick( void )
{
  if ( _server ) {
    return;
  }

  /* send NAT heartbeats */
  if ( _remote == UNKNOWN ) {
    _next_ping_time = Socket::timestamp() + _ping_interval;
    return;
  }

  if ( _next_ping_time < Socket::timestamp() ) {
    SatPayload contents;
    contents.sequence_number = -1;
    contents.ack_number = -1;
    contents.sent_timestamp = Socket::timestamp();
    contents.recv_timestamp = 0;
    contents.sender_id = _ack_id;

    _send.send( Socket::Packet( _remote, contents.str( sizeof( SatPayload ) ) ) );

    _next_ping_time = Socket::timestamp() + _ping_interval;
  }
}

uint64_t Acker::wait_time( void ) const
{
  if ( _server ) {
    return 1000000000;
  }

  int diff = _next_ping_time - Socket::timestamp();
  if ( diff < 0 ) {
    diff = 0;
  }

  return diff;
}
