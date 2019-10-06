/**
 * Copyright 2019 Aaron Robert
 * */
#ifndef SRC_RPC_RPC_H_
#define SRC_RPC_RPC_H_
#include <map>
#include <string>
#include "common/common.h"
#include "util/util.h"

class Proxy {};

class ClientProxy : virtual public Proxy {
 public:
  template <typename RequestType, typename ResponseType>
  ResponseType Call(std::string name, RequestType request);
};

Data* ServeHandle(void* context, Data* const request);

class ServerProxy : virtual public Proxy {
 public:
  // the channel should be binded before proxy init
  explicit ServerProxy(ServerChannel* channel) : channel_(channel) {}
  template <typename RequestType, typename ResponseType, void* func_ptr>
  std::stringstream* Call(std::stringstream* in_data) {
    typedef ResponseType (*call_func_type)(RequestType);
    RequestType req;
    req.ParseFromIstream(in_data);
    ResponseType res = static_cast<call_func_type>(func_ptr)(req);
    std::stringstream* out_data = new std::stringstream;
    res.SerializeToOstream(out_data);
    return out_data;
  }
  template <typename RequestType, typename ResponseType>
  int Register(HashName hashFuncName, ResponseType (*serve_func)(RequestType)) {
    this->func_map[hashFuncName] = &Call<RequestType, ResponseType, serve_func>;
  }
  void Serve();
  friend Data* ServeHandle(void* context, Data* const request);

 private:
  ServerChannel* channel_;
  std::map<HashName, std::stringstream* (*)(std::stringstream*)> func_map;
};

#endif  // SRC_RPC_RPC_H_
