#include <assert.h>
#include <algorithm>
#include <boost/math/distributions/poisson.hpp>
#include <boost/math/distributions/normal.hpp>

#include "process.hh"

using namespace boost::math;

Process::Process( const int maximum_rate, const double s_brownian_motion_rate )
  : _probability_mass_function( maximum_rate + 1, 1.0 / (maximum_rate + 1) ),
    _brownian_motion_rate( s_brownian_motion_rate ),
    _gaussian_cdf( maximum_rate )
{
}

void Process::observe( const double time, const int counts )
{
  /* multiply by likelihood function */
  for ( int rate = 1; rate < (int)_probability_mass_function.size(); rate++ ) {
    _probability_mass_function[ rate ] *= pdf( poisson( rate * time ), counts );
  }

  /* need to handle zero-rate process specially */
  if ( counts == 0 ) {
    /* do nothing */
  } else if ( time > 0 ) {
    _probability_mass_function[ 0 ] = 0;
  }
}

void Process::normalize( void )
{
  double sum( 0 );

  /* find total probability */
  for_each( _probability_mass_function.begin(), _probability_mass_function.end(),
	    [&] ( const double & x ) { sum += x; } );

  /* normalize */
  for_each( _probability_mass_function.begin(), _probability_mass_function.end(),
	    [&] ( double & x ) { x /= sum; } );  
}

void Process::evolve( const double time )
{
  double stddev = _brownian_motion_rate * sqrt( time );

  _gaussian_cdf.calculate( stddev );

  const int max_rate = _probability_mass_function.size() - 1;

  std::vector< double > new_pmf( _probability_mass_function.size() );

  for ( int rate = 0; rate <= max_rate ; rate++ ) {
    double prob = _probability_mass_function[ rate ];

    /* calculate chance of zero */
    new_pmf[ 0 ] += prob * (_gaussian_cdf( 0 + 1 - rate ) - 0);

    /* calculate chance of max */
    new_pmf[ max_rate ] += prob * (1 - _gaussian_cdf( max_rate - rate ));

    /* calculate rest of the intervals */
    for ( int newrate = 1; newrate < max_rate; newrate++ ) {
      new_pmf[ newrate ] += prob * (_gaussian_cdf( newrate + 1 - rate ) - _gaussian_cdf( newrate - rate ));
    }
  }

  _probability_mass_function = new_pmf;
}

Process::GaussianCache::GaussianCache( int max_rate )
  : _offset( max_rate ),
    _stddev( -1 ),
    _cdf( max_rate + 1 + max_rate )
{
}

void Process::GaussianCache::calculate( double stddev )
{
  if ( stddev == _stddev ) {
    /* nothing to do */
    return;
  }

  /* otherwise... */
  _stddev = stddev;
  normal diffdist( 0, stddev );
  for ( unsigned int diff = 0; diff < _cdf.size(); diff++ ) {
    int the_val = diff - _offset;
    _cdf[ diff ] = cdf( diffdist, the_val );
  }
}
