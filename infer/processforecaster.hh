#ifndef PROCESSFORECASTER_HH
#define PROCESSFORECASTER_HH

#include "process.hh"

#include <vector>

class ProcessForecastCache
{
private:
  std::vector< std::vector< double > > _count_probability;

public:
  ProcessForecastCache( const double tick_time, const Process & example, const unsigned int upper_limit );
};

#endif
