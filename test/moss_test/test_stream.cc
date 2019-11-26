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
#include "./stream.h"
#include "gtest/gtest.h"
#include "./interfaces.h"

TEST(Stream, SendSideSimpleSend) {
  auto conn = std::make_shared<moss::Connection>(moss::ConnectionType::kClient);
  auto stream = conn->CreateStream(moss::Directional::kUnidirectional);
  char data[] = "Hello world!";
  stream->WriteData(data, sizeof(data));
  auto conn_cmd = conn->cmdQueue_->PopCmd();
  auto send_data_cmd = std::dynamic_pointer_cast<moss::CmdSendData>(conn_cmd);
  EXPECT_EQ(send_data_cmd->data_len_, sizeof(data));
}
