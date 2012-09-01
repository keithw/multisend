#include "process.hh"
#include "sampledfunction.hh"

#include <vector>
#include <stdio.h>

void print( const SampledFunction & in )
{
  in.for_each( [] ( const double midpoint, const double & value, const int index ) {
      printf( "%f %f\n", midpoint, value );
    } );

  printf( "\n\n" );
}

int main( void )
{
  Process myprocess( 2000, .1, 1, 100 );

  myprocess.normalize();
  myprocess.observe( 0.01, 5 );
  myprocess.normalize();
  print( myprocess.pmf() );

  for ( int amt = 0; amt < 1000; amt++ ) {
    myprocess.evolve( 0.01 );
    myprocess.normalize();
    print( myprocess.pmf() );
  }
}
