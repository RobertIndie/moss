/**
 * Copyright 2019 Aaron Robert
 * */
#include "gtest/gtest.h"
#include "./test.pb.h"
#include "rpc/rpc.h"

TEST(ServerProxy, P2PConnection) {
  UDPClientChannel channel;
  channel.Connect("127.0.0.1", 9877);
  ClientProxy prx(&channel);
  moss_test::SimpleMessage req, res;
  req.set_header("GET");
  req.set_id(1);
  req.set_body("Content");
  prx.Call("P2PTest", &req, &res);
  EXPECT_EQ(res.header(), "GET");
  EXPECT_EQ(res.id(), 1);
  EXPECT_EQ(res.body(), "Content");
}
