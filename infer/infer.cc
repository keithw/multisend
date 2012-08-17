#include "process.hh"

#include <vector>
#include <stdio.h>

void print( const std::vector< double > & in )
{
  for ( unsigned int i = 0; i < in.size(); i++ ) {
    printf( "%d %f\n", i, in[ i ] );
  }
  printf( "\n\n" );
}

int main( void )
{
  Process myprocess( 2000, 1000 );

  int amt = 40;
  myprocess.observe( 0.1, amt );
  myprocess.normalize();
  print( myprocess.pmf() );

  while ( 1 ) {
    myprocess.evolve( 0.1 );
    myprocess.normalize();
    print( myprocess.pmf() );
  }
}
