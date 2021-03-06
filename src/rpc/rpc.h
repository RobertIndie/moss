/**
 * Copyright 2019 Aaron Robert
 * */
#ifndef SRC_RPC_RPC_H_
#define SRC_RPC_RPC_H_
#include <map>
#include <string>
#include "common/common.h"
#include "util/util.h"

#define RECV_DATA_SIZE 1024

struct RequestHeader {
  unsigned int func_name;
};
#define REQUEST_HEADER_LEN sizeof(RequestHeader)
#define REG_FUNC(prx, func_name) prx.Register(#func_name, func_name)

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

class Proxy {};

class ClientProxy : virtual public Proxy {
 public:
  explicit ClientProxy(ClientChannel* channel) : channel_(channel) {}
  template <typename RequestType, typename ResponseType>
  int Call(std::string name, RequestType* request, ResponseType* response) {
    std::stringstream ss_req, ss_res;
    RequestHeader req_hdr;
    auto hash_func_name = BKDRHash(name.c_str());
    req_hdr.func_name = hash_func_name;
    char req_hdr_mem[REQUEST_HEADER_LEN];
    memcpy(req_hdr_mem, &req_hdr, REQUEST_HEADER_LEN);
    ss_req.write(req_hdr_mem, REQUEST_HEADER_LEN);
    ConvertProtoObjToStream(request, &ss_req);
    std::string request_str = ss_req.str();
    Data* request_data = new Data(request_str.c_str(), request_str.length());
    Data* response_data = new Data(RECV_DATA_SIZE);
    DLOG(INFO) << "Call Function" << LOG_VALUE(hash_func_name);
    int ret = this->channel_->Send(request_data, response_data);
    if (ret == -1) {
      return -1;
    }
    DLOG(INFO) << "Call Function Result" << LOG_VALUE(hash_func_name)
               << LOG_VALUE(response_data->len);
    ss_res << std::string(response_data->GetBuff(), response_data->len);
    ConvertStreamToProtoObj(response, &ss_res);
    delete request_data;
    delete response_data;
    return 0;
  }

 private:
  ClientChannel* channel_;
};

Data* ServeHandle(void* context, Data* const request);
class ServerProxy : virtual public Proxy {
 public:
  typedef int (*ServeFuncType)(std::stringstream*, std::stringstream*);
  // the channel should be binded before proxy init
  explicit ServerProxy(ServerChannel* channel) : channel_(channel) {}
  int Register(std::string func_name, ServeFuncType serve_func);
  void Serve();
  friend Data* ServeHandle(void* context, Data* const request);

 private:
  ServerChannel* channel_;
  std::map<HashName, ServeFuncType> func_map;
};

#endif  // SRC_RPC_RPC_H_
