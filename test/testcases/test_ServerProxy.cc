/**
 * Copyright 2019 Aaron Robert
 * */
#include "gtest/gtest.h"
#include "./test.pb.h"
#include "rpc/rpc.h"

class _Fack_ServerChannel : public ServerChannel {
 public:
  int Bind(std::string ip, unsigned short port) {}
  int Serve(void* context, ServeFunc serve_func) {
    moss_test::SimpleMessage req;
    req.set_header("GET");
    req.set_id(12138);
    req.set_body("Login");
    RequestHeader req_hdr{BKDRHash("TestFunction")};
    char* hdr_mem = new char[REQUEST_HEADER_LEN];
    memcpy(hdr_mem, &req_hdr, REQUEST_HEADER_LEN);
    std::stringstream ss;
    std::string req_str;
    req.SerializeToString(&req_str);
    ss << std::string(hdr_mem, REQUEST_HEADER_LEN) << req_str;
    std::string req_str_data = ss.str();
    Data* req_data = new Data(req_str_data.c_str(), req_str_data.size());
    Data* res_data = serve_func(context, req_data);
    std::stringstream r_ss;
    r_ss << std::string(res_data->GetBuff(), res_data->len);
    moss_test::SimpleMessage res;
    res.ParseFromIstream(&r_ss);
    EXPECT_EQ(res.header(), "PASS");
    EXPECT_EQ(res.id(), 12138);
    return 0;
  }
};

int serve(std::stringstream* in_data, std::stringstream* out_data) {
  moss_test::SimpleMessage req, res;
  ConvertStreamToProtoObj(&req, in_data);
  EXPECT_EQ(req.header(), "GET");
  EXPECT_EQ(req.id(), 12138);
  EXPECT_EQ(req.body(), "Login");
  res.set_header("PASS");
  res.set_id(12138);
  ConvertProtoObjToStream(&res, out_data);
  return 0;
}

TEST(ServerProxy, FunctionManagement) {
  _Fack_ServerChannel fackChannel;
  ServerProxy prx(&fackChannel);
  prx.Register("TestFunction", serve);
  prx.Serve();
}
