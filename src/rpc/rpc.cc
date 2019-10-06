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
  unsigned int hashFuncName = 0;
  *ss >> hashFuncName;
  std::stringstream* response_stream = prx->func_map[hashFuncName](ss);
  std::string response_str = response_stream->str();
  Data* response_data = new Data(response_str.c_str(), response_str.length());
  delete ss;
  return response_data;
}

void ServerProxy::Serve() { this->channel_->Serve(this, ServeHandle); }
#pragma endregion
