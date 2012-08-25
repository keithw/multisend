#include <assert.h>
#include <algorithm>
#include <boost/math/distributions/poisson.hpp>
#include <boost/math/distributions/normal.hpp>

#include "process.hh"

using namespace boost::math;

Process::Process( const double maximum_rate, const double s_brownian_motion_rate, const int bins )
  : _probability_mass_function( bins, maximum_rate, 0 ),
    _gaussian( maximum_rate, bins * 2 ),
    _brownian_motion_rate( s_brownian_motion_rate )
{
  normalize();
}

void Process::observe( const double time, const int counts )
{
  /* multiply by likelihood function */
  _probability_mass_function.for_each( [&]
				       ( const double midpoint, double & value )
				       {
					 value *= pdf( poisson( midpoint * time ),
						       counts );
				       } );
}

void Process::normalize( void )
{
  double sum( 0 );

  /* find the total */
  _probability_mass_function.for_each( [&sum]
				       ( const double, const double & value )
				       {
					 sum += value;
				       } );

  /* normalize */
  _probability_mass_function.for_each( [&sum]
				       ( const double, double & value )
				       {
					 value /= sum;
				       } );
}

void Process::evolve( const double time )
{
  /* initialize brownian motion */
  double stddev = _brownian_motion_rate * sqrt( time );
  _gaussian.calculate( stddev );

  /* initialize new pmf */
  SampledFunction new_pmf( _probability_mass_function );
  new_pmf.for_each( [] ( const double, double & value ) { value = 0; } );

  _probability_mass_function.for_each( [&]
				       ( const double old_rate, const double & old_prob )
				       {
					 new_pmf.for_range( old_rate - 5 * stddev,
							    old_rate + 5 * stddev,
							    [&]
							    ( const double new_rate, double & new_prob )
							    {
							      new_prob += old_prob * ( _gaussian.cdf( new_pmf.sample_ceil( new_rate ) - old_rate )
										       - _gaussian.cdf( new_pmf.sample_floor( new_rate ) - old_rate ) );
							    } );
				       } );
  
  _probability_mass_function = new_pmf;
}

Process::GaussianCache::GaussianCache( const double maximum_rate, const int bins )
  : _cdf( bins, maximum_rate, -maximum_rate ),
    _stddev( -1 )
{}

void Process::GaussianCache::calculate( const double s_stddev )
{
  if ( s_stddev == _stddev ) {
    return;
  }

  _stddev = s_stddev;
  normal diffdist( 0, _stddev );

  _cdf.for_each( [&] ( const double x, double & value ) { value = boost::math::cdf( diffdist, x ); } );
}

