/**
 * Copyright 2019 Aaron Robert
 * */
#include "./login.pb.h"
#include "rpc/rpc.h"

Data *handle(void *context, Data *const request) {
  Data *response = new Data(1);
  return response;
}

int Login(std::stringstream *in_data, std::stringstream *out_data) {
  benchmark_test::LoginMessage req;
  benchmark_test::LoginResult res;
  ConvertStreamToProtoObj(&req, in_data);
  if (req.id() == 12138 && req.password() == "123456") {
    res.set_code(0);
    res.set_error("No Error");
  } else {
    res.set_code(-1);
    res.set_error("Error");
  }
  ConvertProtoObjToStream(&res, out_data);
  return 0;
}

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  const char *ip = argv[1];
  int port = atoi(argv[2]);
  if (argc < 3) {
    printf(
        "Usage:\n"
        "example_echosvr [IP] [PORT]\n");
    return -1;
  }
  UDPServerChannel channel;
  channel.Bind(ip, port);
  ServerProxy prx(&channel);
  REG_FUNC(prx, Login);
  prx.Serve();
  return 0;
}
