#include <vector>
#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "process.hh"
#include "processforecaster.hh"

using namespace std;

int main( void )
{
  Process myprocess( 2000, 500, 100 );

  ProcessForecastCache( .01, myprocess, 100 );

  const int interval_ms = 10;

  int current_chunk = -1, count = -1;

  while ( cin.good() ) {
    int ms = 0;
    cin >> ms;

    if ( current_chunk == -1 ) { /* need to initialize */
      current_chunk = ms / interval_ms;
      count = 0;
    }

    assert( ms / interval_ms >= current_chunk );

    while ( current_chunk < ms / interval_ms ) {
      myprocess.evolve( (double)interval_ms / 1000.0 );
      myprocess.observe( (double)interval_ms / 1000.0, count );
      myprocess.normalize();
      current_chunk++;
      count = 0;
      printf( "%d %f\n", current_chunk * interval_ms,
	      myprocess.lower_quantile( .05 ) );
    }

    count++;
  }
}
