/**
 * Copyright 2019 Aaron Robert
 * */
#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <cstdint>
#include <string>
#include "iostream"
#include "util/util.h"

// struct to hold the value:
template <typename T>
struct bits_t {
  T t;
};  // no constructor necessary

// functions to infer type, construct bits_t with a member initialization list
// use a reference to avoid copying. The non-const version lets us extract too
template <typename T>
bits_t<T*> bits(T* t) {
  return bits_t<T*>{t};
}
template <typename T>
bits_t<const T&> bits(const T& t) {
  return bits_t<const T&>{t};
}
// insertion operator to call ::write() on whatever type of stream
template <typename S, typename T>
S& operator<<(S& s, bits_t<T> b) {
  s.write(reinterpret_cast<const char*>(&b.t), sizeof(T));
  return s;
}
// extraction operator to call ::read(), require a pointer here
template <typename S, typename T>
S& operator>>(S& s, bits_t<T*> b) {
  s.read(reinterpret_cast<char*>(b.t), sizeof(T));
  return s;
}

struct Data {
  Data(const char *buff, size_t len) : cbuff(buff), len(len) {}
  explicit Data(size_t len) : len(len) { this->buff = new char[len]; }
  const char *GetBuff() { return buff == nullptr ? cbuff : buff; }
  // TODO(private) : let buff and cbuff be private
  char *buff = nullptr;
  const char *cbuff = nullptr;
  size_t len;
  ~Data() {
    if (buff != nullptr) {
      delete[] buff;
      buff = nullptr;
    }
  }
};
// min retransmit timeout value, in millisecond
const int kRXTMin = 20;
// max retransmit timeout value, in millisecond
const int kRXTMax = 10000;
// max times to retransmit
const int kRXTMaxTimes = 10;
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
  Data *const MakeData(Data *const data);
  msghdr *const GetResult() const;
  Data *const GetData() const;

 private:
  sockaddr_in *_new_sa = nullptr;
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
  virtual int Connect(std::string ip, unsigned short port) {}
  virtual int Send(Data *in_data, Data *out_data) {}
};

typedef Data *(*ServeFunc)(void *context, Data *const request);

class ServerChannel : virtual public Channel {
 public:
  virtual int Bind(std::string ip, unsigned short port) {}
  virtual int Serve(void *context, ServeFunc serve_func) {}
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
  int Serve(void *context, ServeFunc serve_func);
};

#pragma region Factories
class ChannelFactory {
 public:
  virtual ServerChannel *CreateServerChannel(std::string ip,
                                             unsigned int port) = 0;
  virtual ClientChannel *CreateClientChannel(std::string ip,
                                             unsigned int port) = 0;
};

class UDPChannelFactory : public ChannelFactory {
 public:
  ServerChannel *CreateServerChannel(std::string ip, unsigned int port);
  ClientChannel *CreateClientChannel(std::string ip, unsigned int port);
};
#pragma endregion

#endif  // COMMON_COMMON_H_
