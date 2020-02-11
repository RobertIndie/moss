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
#include "./command.h"
#include "./interfaces.h"
#include "./stream.h"
#include "gtest/gtest.h"

class TestConnection : public moss::CommandExecutor {
 public:
  TestConnection() {
    cmdQueue_ =
        std::static_pointer_cast<moss::CommandQueue<moss::CmdConnection>>(
            std::make_shared<moss::CoCmdQueue<moss::CmdConnection>>());
  }
  std::shared_ptr<moss::CommandQueue<moss::CmdConnection>> cmdQueue_;
  void PushCommand(std::shared_ptr<moss::CommandBase> cmd) {
    auto _cmd = std::static_pointer_cast<moss::CmdConnection>(cmd);
    cmdQueue_->PushCmd(std::static_pointer_cast<moss::CmdConnection>(cmd));
  }
};

TEST(Stream, SendSideSimpleSend) {
  auto conn = std::make_shared<TestConnection>();
  auto stream = std::make_shared<moss::Stream>(
      conn.get(), 12345, moss::Initializer::kClient,
      moss::Directional::kUnidirectional);
  char data[] = "Hello world!";
  // Set flow credit
  stream->sendSide_.flow_credit_ = sizeof(data);
  // Test write data
  stream->WriteData(data, sizeof(data));
  auto data_block_cmd =
      std::dynamic_pointer_cast<moss::CmdSendGFL>(conn->cmdQueue_->PopCmd());
  EXPECT_NE(data_block_cmd.get(), nullptr);
  auto send_data_cmd =
      std::dynamic_pointer_cast<moss::CmdSendData>(conn->cmdQueue_->PopCmd());
  EXPECT_EQ(send_data_cmd->send_count_, sizeof(data));
  // std::shared_ptr<char> result_data(new char[sizeof(data)]);
  // send_data_cmd->buffer_->read(result_data.get(), sizeof(data));
  std::shared_ptr<char> result_data(new char[send_data_cmd->send_count_]);
  stream->sendSide_.reader_for_conn_->Read(send_data_cmd->send_count_,
                                           result_data.get());
  EXPECT_EQ(memcmp(data, result_data.get(), sizeof(data)), 0);
  // Test stream flow control -- data block
  stream->WriteData(data, sizeof(data));
  // auto data_cmd =
  //     std::dynamic_pointer_cast<moss::CmdSendData>(conn->cmdQueue_->PopCmd());
  // EXPECT_NE(data_cmd.get(), nullptr);
  auto blocked_cmd =
      std::dynamic_pointer_cast<moss::CmdSendGFL>(conn->cmdQueue_->PopCmd());
  EXPECT_NE(blocked_cmd.get(), nullptr);
  auto frame = std::make_shared<FrameStreamDataBlocked>();
  FrameType frame_type;
  ConvertGFLToFrame(blocked_cmd->gfl_.get(), frame.get(), &frame_type);
  EXPECT_EQ(frame_type, FrameType::kStreamDataBlocked);
  EXPECT_EQ(frame->stream_data_limit, sizeof(data));
  EXPECT_EQ(frame->stream_id, stream->id_);
  // Test final stream
  stream->sendSide_.flow_credit_ += 2 * sizeof(data);  // add flow credit
  stream->WriteData(data, sizeof(data), true);
  auto send_final_data_cmd =
      std::dynamic_pointer_cast<moss::CmdSendData>(conn->cmdQueue_->PopCmd());
  EXPECT_EQ(stream->send_buffer_->is_final_, true);
}
