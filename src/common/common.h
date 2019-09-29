/**
 * Copyright 2019 Aaron Robert
 * */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdint>
#include <string>

struct Data {
  char *data;
  size_t len;
};

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

class Channel {
 public:
  virtual int Connect(std::string ip, unsigned short port) = 0;
  virtual Data *Send(Data *data) = 0;
};

class UDPChannel : public Channel {
 public:
  UDPChannel() { this->sa = new sockaddr_in; }
  int Connect(std::string ip, unsigned short port);
  Data *Send(Data *data);

 private:
  int socket_fd;
  sockaddr_in *sa;
  uint32_t seq;
};
