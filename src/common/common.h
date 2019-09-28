/**
 * Copyright 2019 Aaron Robert
 * */
#include <cstdint>
template <typename TimeType = int32_t, typename CountType = int8_t>
class RTTInfo {
 public:
  TimeType rtt;
  TimeType srtt;
  TimeType rttvar;
  TimeType rto;
  CountType retransmitted_count;
  uint32_t time_base;
};
