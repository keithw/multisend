#ifndef PAYLOAD_HH
#define PAYLOAD_HH

#include <string>

class Payload
{
public:
  uint32_t sequence_number;
  uint64_t sent_timestamp, recv_timestamp;
  int process_id;

  const std::string str( const size_t len ) const;
};

#endif
