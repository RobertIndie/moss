/**
 * Copyright 2019 Aaron Robert
 * */
#include "./test.pb.h"
#include "rpc/rpc.h"

Data *handle(void *context, Data *const request) {
  Data *response = new Data(1);
  return response;
}

int P2PTest(std::stringstream *in_data, std::stringstream *out_data) {
  moss_test::SimpleMessage req, res;
  ConvertStreamToProtoObj(&req, in_data);
  res.set_header(req.header());
  res.set_id(req.id());
  res.set_body(req.body());
  ConvertProtoObjToStream(&res, out_data);
  return 0;
}

int main(int argc, char **argv) {
  InitLogger(argv);
  UDPServerChannel channel;
  channel.Bind("0.0.0.0", 9877);
  ServerProxy prx(&channel);
  REG_FUNC(prx, P2PTest);
  prx.Serve();
  return 0;
}
