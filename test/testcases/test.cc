/**
 * Copyright 2019 Aaron Robert
 * */
#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>
#include <iostream>
#include "../src/util/util.h"
#include "gtest/gtest.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
