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
  Process myprocess( 2000, 1000 );

  int amt = 40;
  myprocess.normalize();
  print( myprocess.pmf() );
  myprocess.evolve( 0.1 );
  myprocess.normalize();
  print( myprocess.pmf() );

  myprocess.observe( 0.1, amt );
  myprocess.normalize();
  print( myprocess.pmf() );

  while ( 1 ) {
    //    myprocess.observe( 0.1, amt );
    myprocess.evolve( 0.1 );
    myprocess.normalize();
    print( myprocess.pmf() );
  }
}
