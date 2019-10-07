/**
 * Copyright 2019 Aaron Robert
 * */
#include "gtest/gtest.h"
#include "rpc/rpc.h"

class _Fack_ServerChannel : public ServerChannel {
 public:
  int Bind(std::string ip, unsigned short port) {}
  int Serve(void *context, ServeFunc serve_func) {
      
  }
};

TEST(ServerProxy, FunctionManagement) { ServerProxy prx; }
