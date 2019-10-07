/**
 * Copyright 2019 Aaron Robert
 * */
#include "rpc/rpc.h"

#pragma region ServerProxy
Data* ServeHandle(void* context, Data* const request) {
  ServerProxy* prx = reinterpret_cast<ServerProxy*>(context);
  std::string str(request->buff, request->len);
  std::stringstream* ss = new std::stringstream;
  *ss << str;
  RequestHeader req_hdr;
  char* req_hdr_mem = new char[REQUEST_HEADER_LEN];
  ss->read(req_hdr_mem, REQUEST_HEADER_LEN);
  memcpy(&req_hdr, req_hdr_mem, REQUEST_HEADER_LEN);
  std::stringstream* response_stream = prx->func_map[req_hdr.func_name](ss);
  std::string response_str = response_stream->str();
  Data* response_data = new Data(response_str.c_str(), response_str.length());
  delete ss;
  delete req_hdr_mem;
  return response_data;
}

void ServerProxy::Serve() { this->channel_->Serve(this, ServeHandle); }
#pragma endregion
