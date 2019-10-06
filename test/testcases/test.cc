/**
 * Copyright 2019 Aaron Robert
 * */
#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>
#include <iostream>
#include "../src/util/util.h"
#include "gtest/gtest.h"

TEST(TestSuite, TestName) {
  ASSERT_TRUE(true);
  EXPECT_TRUE(false);
  ASSERT_TRUE(false);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
