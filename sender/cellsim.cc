#include <unistd.h>
#include <string>
#include <assert.h>
#include <list>
#include <stdio.h>
#include <queue>
#include <limits.h>

#include "select.h"
#include "timestamp.h"
#include "packetsocket.hh"

using namespace std;

class DelayQueue
{
private:
  class DelayedPacket
  {
  public:
    uint64_t entry_time;
    uint64_t release_time;
    string contents;

    DelayedPacket( uint64_t s_e, uint64_t s_r, const string & s_c )
      : entry_time( s_e ), release_time( s_r ), contents( s_c ) {}
  };

  class PartialPacket
  {
  public:
    int bytes_earned;
    DelayedPacket packet;
    
    PartialPacket( int s_b_e, const DelayedPacket & s_packet ) : bytes_earned( s_b_e ), packet( s_packet ) {}
  };

  static const int SERVICE_PACKET_SIZE = 1514;

  uint64_t convert_timestamp( const uint64_t absolute_timestamp ) const { return absolute_timestamp - _base_timestamp; }

  const string _name;

  queue< DelayedPacket > _delay;
  queue< DelayedPacket > _pdp;
  queue< PartialPacket > _limbo;

  queue< uint64_t > _schedule;

  vector< string > _delivered;

  const uint64_t _ms_delay;
  const float _loss_rate;

  uint64_t _total_bytes;
  uint64_t _used_bytes;

  uint64_t _queued_bytes;
  uint64_t _bin_sec;

  uint64_t _base_timestamp;

  uint32_t _packets_added;
  uint32_t _packets_dropped;
  string _file_name;

  bool _printing;

  void tick( void );


public:
  DelayQueue( const string & s_name, const uint64_t s_ms_delay, const string filename, const uint64_t base_timestamp, const float loss_rate, bool s_printing = false );

  int wait_time( void );
  vector< string > read( void );
  void write( const string & packet );
  void schedule_from_file( const uint64_t base_timestamp );
};

DelayQueue::DelayQueue( const string & s_name, const uint64_t s_ms_delay, const string filename, const uint64_t base_timestamp, const float loss_rate, bool s_printing )
  : _name( s_name ),
    _delay(),
    _pdp(),
    _limbo(),
    _schedule(),
    _delivered(),
    _ms_delay( s_ms_delay ),
    _loss_rate(loss_rate),
    _total_bytes( 0 ),
    _used_bytes( 0 ),
    _queued_bytes( 0 ),
    _bin_sec( base_timestamp / 1000 ),
    _base_timestamp( base_timestamp ),
    _packets_added ( 0 ),
    _packets_dropped( 0 ),
    _file_name( filename ),
    _printing( s_printing )
{
  /* Read schedule from file */
  schedule_from_file( base_timestamp );

  /* Initialize seed for probabilistic loss model */
  srand(0);
  fprintf( stderr, "Initialized %s queue with %d services.\n", filename.c_str(), (int)_schedule.size() );
}

void DelayQueue::schedule_from_file( const uint64_t base_timestamp ) 
{
  FILE *f = fopen( _file_name.c_str(), "r" );
  if ( f == NULL ) {
    perror( "fopen" );
    exit( 1 );
  }

  /* Only populate when the schedule is empty */
  assert( _schedule.empty() );

  while ( 1 ) {
    uint64_t ms;
    int num_matched = fscanf( f, "%lu\n", &ms );
    if ( num_matched != 1 ) {
      break;
    }

    ms += base_timestamp;

    if ( !_schedule.empty() ) {
      assert( ms >= _schedule.back() );
    }

    _schedule.push( ms );
  }
  fclose( f );
}

int DelayQueue::wait_time( void )
{
  int delay_wait = INT_MAX, schedule_wait = INT_MAX;

  uint64_t now = timestamp();

  tick();

  if ( !_delay.empty() ) {
    delay_wait = _delay.front().release_time - now;
    if ( delay_wait < 0 ) {
      delay_wait = 0;
    }
  }

  if ( !_schedule.empty() ) {
    schedule_wait = _schedule.front() - now;
    assert( schedule_wait >= 0 );
  }

  return min( delay_wait, schedule_wait );
}

vector< string > DelayQueue::read( void )
{
  tick();

  vector< string > ret( _delivered );
  _delivered.clear();

  return ret;
}

void DelayQueue::write( const string & packet )
{
  float r= rand()/(float)RAND_MAX;
  _packets_added++;
  if (r < _loss_rate) {
   _packets_dropped++;
   fprintf(stderr, "%s , Stochastic drop of packet, _packets_added so far %d , _packets_dropped %d , drop rate %f \n",
                  _name.c_str(), _packets_added,_packets_dropped , (float)_packets_dropped/(float) _packets_added );
  }
  else {
    uint64_t now( timestamp() );
    DelayedPacket p( now, now + _ms_delay, packet );
    _delay.push( p );
    _queued_bytes=_queued_bytes+packet.size();
  }
}

void DelayQueue::tick( void )
{
  uint64_t now = timestamp();

  /* If the schedule is empty, repopulate it */
  if ( _schedule.empty() ) {
    schedule_from_file( now );
  }

  /* move packets from end of delay to PDP */
  while ( (!_delay.empty())
	  && (_delay.front().release_time <= now) ) {
    _pdp.push( _delay.front() );
    _delay.pop();
  }

  /* execute packet delivery schedule */
  while ( (!_schedule.empty())
	  && (_schedule.front() <= now) ) {
    /* grab a PDO */
    const uint64_t pdo_time = _schedule.front();
    _schedule.pop();
    int bytes_to_play_with = SERVICE_PACKET_SIZE;

    /* execute limbo queue first */
    if ( !_limbo.empty() ) {
      if ( _limbo.front().bytes_earned + bytes_to_play_with >= (int)_limbo.front().packet.contents.size() ) {
	/* deliver packet */
	_total_bytes += _limbo.front().packet.contents.size();
	_used_bytes += _limbo.front().packet.contents.size();

	if ( _printing ) {
	  printf( "%s %lu delivery %d %lu leftover\n", _name.c_str(), convert_timestamp( pdo_time ), int(pdo_time - _limbo.front().packet.entry_time), _limbo.front().packet.contents.size() );
	}

	_delivered.push_back( _limbo.front().packet.contents );
	bytes_to_play_with -= (_limbo.front().packet.contents.size() - _limbo.front().bytes_earned);
	assert( bytes_to_play_with >= 0 );
	_limbo.pop();
	assert( _limbo.empty() );
      } else {
	_limbo.front().bytes_earned += bytes_to_play_with;
	bytes_to_play_with = 0;
	assert( _limbo.front().bytes_earned < (int)_limbo.front().packet.contents.size() );
      }
    }
    
    /* execute regular queue */
    while ( bytes_to_play_with > 0 ) {
      assert( _limbo.empty() );

      /* will this be an underflow? */
      if ( _pdp.empty() ) {
	/* underflow */
	if ( _printing ) {
	  printf( "%s %lu underflow %d\n", _name.c_str(), convert_timestamp( pdo_time ), bytes_to_play_with );
	}
	_total_bytes += bytes_to_play_with;
	bytes_to_play_with = 0;
      } else {
	/* dequeue whole and/or partial packet */
	DelayedPacket packet = _pdp.front();
	_pdp.pop();
	if ( bytes_to_play_with >= (int)packet.contents.size() ) {
	  /* deliver whole packet */
	  _total_bytes += packet.contents.size();
	  _used_bytes += packet.contents.size();

	  if ( _printing ) {
	    printf( "%s %lu delivery %d %lu\n", _name.c_str(), convert_timestamp( pdo_time ), int(pdo_time - packet.entry_time), packet.contents.size() );
	  }

	  _delivered.push_back( packet.contents );
	  bytes_to_play_with -= packet.contents.size();
	} else {
	  /* put packet in limbo */
	  assert( _limbo.empty() );

	  assert( bytes_to_play_with < (int)packet.contents.size() );

	  PartialPacket limbo_packet( bytes_to_play_with, packet );
	  
	  _limbo.push( limbo_packet );
	  bytes_to_play_with -= _limbo.front().bytes_earned;
	  assert( bytes_to_play_with == 0 );
	}
      }
    }
  }

  while ( now / 1000 > _bin_sec ) {
    fprintf( stderr, "%s %ld %ld / %ld = %.1f %% %ld \n", _name.c_str(), _bin_sec - (_base_timestamp / 1000), _used_bytes, _total_bytes, 100.0 * _used_bytes / (double) _total_bytes , _queued_bytes );
    _total_bytes = 0;
    _used_bytes = 0;
    _queued_bytes = 0;
    _bin_sec++;
  }
}

int main( int argc, char *argv[] )
{
  assert( argc == 7 );

  const string up_filename = argv[ 1 ];
  const string down_filename = argv[ 2 ];
  const string client_mac = argv[ 3 ];
  const double loss_rate = atof( argv[ 4 ] );

  const string internet_side_interface = argv[ 5 ];
  const string client_side_interface   = argv[ 6 ];

  PacketSocket internet_side( internet_side_interface );
  PacketSocket client_side( client_side_interface );

  /* Read in schedule */
  uint64_t now = timestamp();
  DelayQueue uplink( "uplink", 20, up_filename, now , loss_rate );
  DelayQueue downlink( "downlink", 20, down_filename, now , loss_rate, true );

  Select &sel = Select::get_instance();
  sel.add_fd( internet_side.fd() );
  sel.add_fd( client_side.fd() );

  while ( 1 ) {
    int wait_time = min( uplink.wait_time(), downlink.wait_time() );
    int active_fds = sel.select( wait_time );
    if ( active_fds < 0 ) {
      perror( "select" );
      exit( 1 );
    }

    if ( sel.read( client_side.fd() ) ) {
      for ( const auto & it : client_side.recv_raw() ) {
	uplink.write( it );
      }
    }

    if ( sel.read( internet_side.fd() ) ) {
      for ( const auto & it : internet_side.recv_raw() ) {
	downlink.write( it );
      }
    }

    for ( const auto & it : uplink.read() ) {
      internet_side.send_raw( it );
    }

    for ( const auto & it : downlink.read() ) {
      client_side.send_raw( it );
    }
  }
}
