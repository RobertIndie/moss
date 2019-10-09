/**
 * Copyright 2019 Aaron Robert
 * */
#include "rpc/rpc.h"

#pragma region ClientProxy

#pragma endregion

#pragma region ServerProxy
Data* ServeHandle(void* context, Data* const request) {
  ServerProxy* prx = reinterpret_cast<ServerProxy*>(context);
  std::stringstream ss_req, ss_res;
  const char* req_buff = request->GetBuff();
  for (int i = 0; i < request->len; i++) {
    ss_req << req_buff[i];
  }
  RequestHeader req_hdr;
  char req_hdr_mem[REQUEST_HEADER_LEN];
  ss_req.read(req_hdr_mem, REQUEST_HEADER_LEN);
  memcpy(&req_hdr, req_hdr_mem, REQUEST_HEADER_LEN);
  prx->func_map[req_hdr.func_name](&ss_req, &ss_res);
  std::string response_str = ss_res.str();
  Data* response_data = new Data(response_str.c_str(), response_str.length());
  return response_data;
}

int ServerProxy::Register(std::string func_name, ServeFuncType serve_func) {
  this->func_map[BKDRHash(func_name.c_str())] = serve_func;
  return 0;
}

void ServerProxy::Serve() { this->channel_->Serve(this, ServeHandle); }
#pragma endregion
