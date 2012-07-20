#ifndef HIST_HH
#define HIST_HH

#include <vector>

class Histogram
{
private:
  std::vector<int> storage;
  const int bin_width;

public:
  Histogram( const int s_bin_width );
  void record( const int sample );
  void print( void ) const;
};

#endif
