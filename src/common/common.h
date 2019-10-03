/**
 * Copyright 2019 Aaron Robert
 * */
#ifndef SRC_COMMON_COMMON_H_
#define SRC_COMMON_COMMON_H_
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <cstdint>
#include <string>
#include "iostream"
#include "util/util.h"

struct Data {
  Data(char *buff, size_t len) : buff(buff), len(len), isFreeMem(false) {}
  explicit Data(size_t len) : isFreeMem(true) { this->buff = new char[len]; }
  char *buff;
  size_t len;
  ~Data() {
    if (isFreeMem && buff != nullptr) {
      delete[] buff;
      buff = nullptr;
    }
  }

 private:
  bool isFreeMem = false;
};
// min retransmit timeout value, in millisecond
const int kRXTMin = 20;
// max retransmit timeout value, in millisecond
const int kRXTMax = 10000;
// max times to retransmit
const int kRXTMaxTimes = 3;
class RTTInfo {
 public:
  float rtt;
  float srtt;
  float rttvar;
  int retransmitted_count;
  float rto;
  int64_t time_base;
  RTTInfo() { this->Init(); }
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

struct Header {
  uint32_t seq;
  uint32_t ts;
};

// Builder pattern
class PacketBuilder {
 public:
  explicit PacketBuilder(sockaddr_in *sa);
  ~PacketBuilder();
  Header *MakeHeader(uint32_t seq = 0, uint32_t ts = 0);
  Data *MakeData(char *data = nullptr,
                 size_t data_size = 0);  // if data is NULL, then PacketBuilder
                                         // will create memory for data.
  Data *MakeData(Data *data);
  msghdr *GetResult() const;

 private:
  iovec *_iov = nullptr;
  Header *_header = nullptr;
  Data *_data = nullptr;
  msghdr *result = nullptr;
};

class Channel {
 public:
  Channel() {}

 protected:
  sockaddr_in *sa_ = new sockaddr_in;
  int socket_fd_;
};

class ClientChannel : virtual public Channel {
 public:
  virtual int Connect(std::string ip, unsigned short port) = 0;
  virtual int Send(Data *in_data, Data *out_data) = 0;
};

typedef Data *(*ServeFunc)(Data *);

class ServerChannel : virtual public Channel {
 public:
  virtual int Bind(std::string ip, unsigned short port) = 0;
  virtual int Serve(ServeFunc serve_func) = 0;
};

class UDPChannel : virtual public Channel {
 protected:
  void SocketConnect(std::string ip, unsigned short port);
  void SocketBind();
};

class UDPClientChannel : public ClientChannel, public UDPChannel {
 public:
  UDPClientChannel() : Channel() {}
  int Connect(std::string ip, unsigned short port);
  int Send(Data *, Data *);

 private:
  bool reinit_rtt = false;
  RTTInfo rtt_info_;
  uint32_t seq_ = 0;
};

class UDPServerChannel : public ServerChannel, public UDPChannel {
 public:
  UDPServerChannel() : Channel() {}
  int Bind(std::string ip, unsigned short port);
  int Serve(ServeFunc serve_func)__attribute__((optimize(0)));
};

#endif  // SRC_COMMON_COMMON_H_
