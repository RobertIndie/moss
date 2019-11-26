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
#include "./connection.h"
#include "./interfaces.h"
#include "./stream.h"
#include "gtest/gtest.h"

TEST(Stream, SendSideSimpleSend) {
  auto conn = std::make_shared<moss::Connection>(moss::ConnectionType::kClient);
  auto stream = conn->CreateStream(moss::Directional::kUnidirectional);
  char data[] = "Hello world!";
  // Set flow credit
  stream->sendSide_.flow_credit_ = sizeof(data);
  // Test write data
  stream->WriteData(data, sizeof(data));
  auto send_data_cmd =
      std::dynamic_pointer_cast<moss::CmdSendData>(conn->cmdQueue_->PopCmd());
  EXPECT_EQ(send_data_cmd->data_len_, sizeof(data));
  std::shared_ptr<char> result_data(new char[sizeof(data)]);
  send_data_cmd->buffer_->read(result_data.get(), sizeof(data));
  EXPECT_EQ(memcmp(data, result_data.get(), sizeof(data)), 0);
  // Test stream flow control -- data block
  // stream->WriteData(data, sizeof(data));
  // auto blocked_cmd =
  //     std::dynamic_pointer_cast<moss::CmdSendGFL>(conn->cmdQueue_->PopCmd());
  // EXPECT_EQ(blocked_cmd->gfl_->frame_type, FrameType::kStreamDataBlocked);
  // auto frame = std::make_shared<FrameStreamDataBlocked>();
}
