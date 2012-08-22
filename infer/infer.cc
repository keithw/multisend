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
  Process myprocess( 2000, 1000, 100 );

  for ( int amt = 0; amt < 1000; amt++ ) {
    myprocess.observe( 0.01, amt );
    myprocess.evolve( 0.01 );
    myprocess.normalize();
    amt++;
  }
}
