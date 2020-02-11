//
// Copyright (C) 2019 Linkworld Open Team
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https: //www.gnu.org/licenses/>.

#ifndef __MOSS_TEST
#define __MOSS_TEST
#endif
#include "./routine.h"
#include "gtest/gtest.h"

struct TestParam {
  std::shared_ptr<moss::AsynRoutine> routine;
  int times;
  int result;
};

void *func(void *param) {
  auto param_ = static_cast<TestParam *>(param);
  while (true) {
    param_->result++;
    param_->routine->Suspend();
  }
}

TEST(Routine, Coroutine) {
  auto coroutine = std::make_shared<moss::Coroutine>();
  TestParam param;
  param.routine = coroutine;
  param.times = 2;
  param.result = 0;
  coroutine->Init(func, &param);
  for (int i = 0; i < param.times; i++) {
    coroutine->Resume();
  }
  EXPECT_EQ(param.result, param.times);
}
