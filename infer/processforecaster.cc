#include <assert.h>
#include <stdio.h>

#include "processforecaster.hh"

ProcessForecastCache::ProcessForecastCache( const double tick_time,
					    const Process & example,
					    const unsigned int upper_limit )
  : _count_probability()
{
  /* step 1: make the component processes */
  std::vector< Process > components;

  example.pmf().for_each( [&] ( const double midpoint, const double & value, unsigned int index )
			  {
			    Process component( example );
			    component.set_certain( midpoint );
			    assert( components.size() == index );
			    assert( example.pmf().index( midpoint ) == index );
			    components.push_back( component );
			  } );

  /* step 2: for each process component, find the distribution of arrivals */
  for ( auto it = components.begin(); it != components.end(); it++ ) {
    std::vector< double > this_count_probability;
    double total = 0.0;
    for ( unsigned int i = 0; i < upper_limit; i++ ) {
      const double prob = it->count_probability( tick_time, i );
      this_count_probability.push_back( prob );
      total += prob;
    }
    assert( total < 1.0 + (1e-10) );
    this_count_probability.push_back( 1.0 - total );

    /* append to cache */
    _count_probability.push_back( this_count_probability );
  }
}
