#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "sampledfunction.hh"

class Process
{
private:
  class GaussianCache {
  private:
    SampledFunction _cdf;
    double _stddev;

  public:
    GaussianCache( const double maximum_rate, const int bins );
    void calculate( const double s_stddev );
    double cdf( const double x ) const { return _cdf[ x ]; }
  };

  SampledFunction _probability_mass_function;
  GaussianCache _gaussian;

  const double _brownian_motion_rate; /* stddev of difference after one second */

public:
  Process( const double maximum_rate, const double s_brownian_motion_rate, const int bins );

  /* apply brownian motion */
  void evolve( const double time );

  /* update from new observation */
  void observe( const double time, const int counts );

  /* make pmf sum to unity */
  void normalize( void );

  double lower_quantile( const double x ) const { return _probability_mass_function.lower_quantile( x ); }
};

#endif
