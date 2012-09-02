#ifndef CELLQUEUE_HH
#define CELLQUEUE_HH

#include <queue>

class CellQueue
{
private:
  std::queue< double > _packets;

  int _opportunities, _deliveries;

public:
  CellQueue() : _packets(), _opportunities( 0 ), _deliveries( 0 ) {}

  void send( const double time );
  bool recv( const double time );
  unsigned int size( void ) const { return _packets.size(); }
};

#endif
