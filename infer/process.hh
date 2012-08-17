#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <vector>

class Process
{
private:
  /* index: (integer) packets per second */
  std::vector< double > _probability_mass_function;

  const double _brownian_motion_rate; /* stddev of difference after one second */

  class GaussianCache {
  private:
    int _offset;
    double _stddev;
    std::vector< double > _cdf;

    const double & cached_cdf( int val );

  public:
    GaussianCache( int max_rate );
    const double & operator()( double stddev, int val );
  };
  
  GaussianCache _gaussian_cdf;

public:
  Process( const int maximum_rate, const double s_brownian_motion_rate );

  /* apply brownian motion */
  void evolve( const double time );

  /* update from new observation */
  void observe( const double time, const int counts );

  /* make pmf sum to unity */
  void normalize( void );

  const std::vector< double > & pmf( void ) const { return _probability_mass_function; }
};

#endif
