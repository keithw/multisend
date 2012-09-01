#ifndef CELLQUEUE_HH
#define CELLQUEUE_HH

#include <queue>

class CellQueue
{
private:
  std::queue< int > _packets;

  int _opportunities, _deliveries;

public:
  CellQueue() : _packets(), _opportunities( 0 ), _deliveries( 0 ) {}

  void send( const int time );
  void recv( const int time );
};

#endif
