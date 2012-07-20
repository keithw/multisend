#include <assert.h>
#include <algorithm>

#include "hist.hh"

Histogram::Histogram( const int s_bin_width )
  : storage(),
    bin_width( s_bin_width )
{
}

void Histogram::record( const int sample )
{
  /* use offset bins centered on bin_width * k */
  int sample_transformed = (sample + (bin_width / 2)) / bin_width;
  assert( sample_transformed >= 0 );
  if ( (int)storage.size() <= sample_transformed ) {
    storage.resize( sample_transformed + 1 );
  }
  storage.at( sample_transformed )++;
}

void Histogram::print( void ) const
{
  for ( size_t i = 0; i < storage.size(); i++ ) {
    printf( "%d %d\n", (int) i * bin_width, storage[ i ] );
  }
}
