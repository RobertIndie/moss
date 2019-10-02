/**
 * Copyright 2019 Aaron Robert
 * */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <cstdint>
#include <string>
#include "util/util.h"

struct Data {
  char *buff;
  size_t len;
};
// min retransmit timeout value, in millisecond
const int kRXTMin = 20;
// max retransmit timeout value, in millisecond
const int kRXTMax = 10000;
// max times to retransmit
const int kRXTMaxTimes = 3;
class RTTInfo {
 public:
  float rtt = 0;
  float srtt = 0;
  float rttvar = 0.75;
  int retransmitted_count;
  float rto = 0;
  int64_t time_base;
  RTTInfo() {
    this->Init();
  }  // The constructor of the template class can only be implemented in the
     // header file.
  void Init();
  void NewPack();
  // get and minmax rto before start sending msg, do not set rto = GetRTO() in
  // sending
  float GetRTO();
  template <typename TimeType = uint32_t>
  TimeType GetRelativeTs();
  template <typename TimeType = uint32_t>
  TimeType Start();
  // Called when send packet timeout, and check if retransmitted_count is more
  // than max times to retransmit
  // if retransmiited_count > max retransmit times : return -1,stop packet
  // sending, otherwise return 0,continue sending
  int Timeout();
  template <typename TimeType = uint32_t>
  void Stop(TimeType);
};

struct hdr {
  uint32_t seq;
  uint32_t ts;
};

class Channel {
 public:
  virtual int Connect(std::string ip, unsigned short port) = 0;
  // virtual int Listen(std::string ip, unsigned short port) = 0;
  virtual int Send(Data *in_data, Data *out_data) = 0;
};

class UDPChannel : public Channel {
 public:
  UDPChannel() {}
  int Connect(std::string ip, unsigned short port);
  // int Listen(std::string ip, unsigned short port) = 0;
  int Send(Data *, Data *);

 private:
  bool reinit_rtt = false;
  RTTInfo rtt_info_;
  int socket_fd_;
  sockaddr_in *sa_ = new sockaddr_in;
  uint32_t seq_ = 0;
};
