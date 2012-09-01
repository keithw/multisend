#include <vector>
#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "receiver.hh"

using namespace std;

int main( void )
{
  Receiver *receiver = NULL;

  //  const double FRAME_INTERVAL = 0.1;
  //  double last_frame = -1;

  while ( cin.good() ) {
    int ms = -1;
    cin >> ms;

    double time = ms / 1000.0;

    if ( !receiver ) {
      receiver = new Receiver( time );
      //      last_frame = time;
    }

    receiver->advance_to( time );
    receiver->recv();
    receiver->forecast();
  }
}
