/**
 * Copyright 2019 Aaron Robert
 * */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <cstdint>
#include <string>
#include "util/util.h"

struct Data {
  char *buff;
  size_t len;
};

template <typename TimeType = int32_t, typename CountType = int8_t>
class RTTInfo {
 public:
  TimeType rtt = 0;
  TimeType srtt = 0;
  TimeType rttvar = 0.75;
  TimeType rto;
  CountType retransmitted_count;
  RTTInfo();
  int64_t time_base;
  void NewPack();
  TimeType MinMax();
};

struct hdr {
  uint32_t seq;
  uint32_t ts;
};

class Channel {
 public:
  virtual int Connect(std::string ip, unsigned short port) = 0;
  virtual Data *Send(Data *in_data, Data *out_data) = 0;
};

class UDPChannel : public Channel {
 public:
  UDPChannel() {}
  int Connect(std::string ip, unsigned short port);
  Data *Send(Data *, Data *);

 private:
  RTTInfo<> rtt_info_;
  int socket_fd_;
  sockaddr_in *sa_ = new sockaddr_in;
  uint32_t seq_ = 0;
};
