#include "process.hh"

#include <vector>
#include <stdio.h>

void print( const Process::SampledFunction & in )
{
  in.for_each( [] ( const double midpoint, const double & value ) {
      printf( "%f %f\n", midpoint, value );
    } );

  printf( "\n\n" );
}

int main( void )
{
  Process myprocess( 2000, 100, 100 );

  for ( int i = 0; i < 2000; i++ ) {
    int amt = 10;
    if ( i > 1000 ) {
      amt = 0;
    }
    myprocess.observe( .01, amt );
    myprocess.evolve( .01 );
    myprocess.normalize();
    printf( "%d %f %f\n", i, myprocess.lower_quantile( .05 ), myprocess.lower_quantile( .5 ) );
  }
}
