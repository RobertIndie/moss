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
#include "gtest/gtest.h"
#include "util/databuffer.h"

TEST(DataBuffer, DPTR) {
  DataBuffer buffer;
  auto a = buffer.NewReader();
  auto b = buffer.NewReader();
  buffer.writer_pos_ = 5;
  // test Move
  EXPECT_EQ(DPTR::Move(&buffer, a->ptr_, -1), 7);
  EXPECT_EQ(DPTR::Move(&buffer, a->ptr_, 7), 7);
  EXPECT_EQ(DPTR::Move(&buffer, a->ptr_, 8), 0);
  *a += 1;
  EXPECT_EQ(a->ptr_, 1);
  EXPECT_EQ(*a > *b, true);
}

TEST(DataBuffer, Write) {
  DataBuffer buffer(8, false);
  auto reader = buffer.NewReader();
  char write_data[5] = {1, 2, 3, 4, 5};
  buffer.Write(5, write_data);
  // EXPECT_EQ(memcmp(buffer.block_->buffer_, write_data, sizeof(write_data)),
  // 0);
  buffer.Write(5, write_data);
  EXPECT_EQ(buffer.cap_size_, 16);
}

TEST(DataBuffer, Read) {
  DataBuffer buffer;
  auto a = buffer.NewReader();
  buffer.writer_pos_ = 5;
  EXPECT_EQ(a->Read(3), 3);
}
