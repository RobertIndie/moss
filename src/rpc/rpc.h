/**
 * Copyright 2019 Aaron Robert
 * */
#ifndef SRC_RPC_RPC_H_
#define SRC_RPC_RPC_H_
#include <map>
#include <string>
#include "common/common.h"
#include "util/util.h"

struct RequestHeader {
  unsigned int func_name;
};
#define REQUEST_HEADER_LEN sizeof(RequestHeader)

class Proxy {};

class ClientProxy : virtual public Proxy {
 public:
  template <typename RequestType, typename ResponseType>
  ResponseType Call(std::string name, RequestType request);
};

Data* ServeHandle(void* context, Data* const request);
class ServerProxy : virtual public Proxy {
 public:
  typedef int (*ServeFuncType)(std::stringstream*, std::stringstream*);
  // the channel should be binded before proxy init
  explicit ServerProxy(ServerChannel* channel) : channel_(channel) {}
  int Register(HashName func_name, ServeFuncType serve_func);
  void Serve();
  friend Data* ServeHandle(void* context, Data* const request);

 private:
  ServerChannel* channel_;
  std::map<HashName, ServeFuncType> func_map;
};

template <typename ProtoObjType>
bool ConvertStreamToProtoObj(ProtoObjType* protoObj,
                             std::stringstream* stream) {
  return protoObj->ParseFromIstream(stream);
}

template <typename ProtoObjType>
bool ConvertProtoObjToStream(ProtoObjType* protoObj,
                             std::stringstream* stream) {
  return protoObj->SerializeToOstream(stream);
}

#endif  // SRC_RPC_RPC_H_
