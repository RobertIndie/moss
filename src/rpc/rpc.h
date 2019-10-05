/**
 * Copyright 2019 Aaron Robert
 * */
#ifndef SRC_RPC_RPC_H_
#define SRC_RPC_RPC_H_
#include <map>
#include <string>
#include "common/common.h"
#include "util/util.h"

class Proxy {
 public:
  int Init(std::string ip, unsigned short port);
};

class ClientProxy : virtual public Proxy {
 public:
  template <typename RequestType, typename ResponseType>
  ResponseType Call(std::string name, RequestType request);
};

class ServerProxy : virtual public Proxy {
 public:
  template <typename RequestType, typename ResponseType>
  std::stringstream Call(std::stringstream in_data, void* func_ptr) {
    typedef ResponseType (*call_func_type)(RequestType);
    RequestType req;
    req.ParseFromIstream(&in_data);
    ResponseType res = static_cast<call_func_type>(func_ptr)(req);
    std::stringstream out_data;
    res.SerializeToOstream(&out_data);
    return out_data;
  }
  template <typename RequestType, typename ResponseType>
  int Register(HashName hashFuncName, ResponseType (*serve_func)(RequestType));
  void Serve();

 private:
  std::map<HashName, std::stringstream (*)(std::stringstream, void*)> func_map;
};

#endif  // SRC_RPC_RPC_H_
