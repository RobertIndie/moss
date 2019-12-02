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

#include "fsm/fsm.h"
#include "gtest/gtest.h"

TEST(FSM, FSMTest) {
  FSM machine(1);
  int status = 0;
  machine.When(1, FSM::transition_t(1, 2));
  machine.When(1, FSM::transition_t(2, 3));
  machine.When(2, FSM::transition_t(3, 1));
  machine.On(1, [&status, &machine]() -> int {
    status = 1;
    machine.Trigger(1);
  });
  machine.On(2, [&status, &machine]() -> int {
    status = 2;
    machine.Trigger(1);
  });
  machine.On(3, [&status, &machine]() -> int {
    status = 3;
    machine.Trigger(2);
  });
  EXPECT_EQ(machine.GetState(), 1);
  machine.Run();
  EXPECT_EQ(machine.GetState(), 2);
  EXPECT_EQ(status, 1);
  machine.Run();
  EXPECT_EQ(status, 2);
  EXPECT_EQ(machine.GetState(), 3);
  machine.Run();
  EXPECT_EQ(status, 3);
  EXPECT_EQ(machine.GetState(), 1);
}
