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

  myprocess.normalize();

  const int predict_ticks = 10;
  const int interval_ms = 10;

  ProcessForecastInterval forecastr( (double)interval_ms / 1000.0, myprocess, 30, predict_ticks );

  int current_chunk = -1, count = -1;

  int predicted_counts = 0;
  int predict_end = 0;
  int actual_counts = 0;

  fprintf( stderr, "Ready...\n" );

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

      if ( current_chunk == predict_end ) {
	printf( "%d actual = %d predicted = %d\n", current_chunk * interval_ms,
		actual_counts,
		predicted_counts );

	predict_end = current_chunk + predict_ticks;
	actual_counts = 0;
	predicted_counts = forecastr.lower_quantile( myprocess, 0.05 );
      }

      current_chunk++;
      count = 0;
    }

    count++;
    actual_counts++;
  }
}
