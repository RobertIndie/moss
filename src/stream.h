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

#ifndef STREAM_H_
#define STREAM_H_

#include <memory>
#include <queue>
#include <sstream>
#include "./co_routine.h"
#include "./command.h"
#include "./frame.h"
#include "./fsm/fsm.h"
#include "./interfaces.h"

namespace moss {

class StreamSide {
 public:
  StreamSide() : fsm_(0) {}

 protected:
  FSM fsm_;
};

class Stream;
class SendSide;

class SendSide : public StreamSide, public CommandExecutor {
 public:
  struct CmdSendSide : public CommandBase {
    int Execute(void* const arg) {
      auto sendSide = static_cast<SendSide* const>(arg);
      return Call(sendSide);
    }

   protected:
    virtual int Call(SendSide* const sendSide) = 0;
  };
  struct CmdWriteData : public CmdSendSide {
   public:
    std::size_t GetHash() const { return typeid(this).hash_code(); }
    explicit CmdWriteData(std::shared_ptr<std::stringstream> data)
        : data_(data) {}

   protected:
    std::shared_ptr<std::stringstream> data_;
    int Call(std::shared_ptr<SendSide> sendSide) { sendSide->WriteData(data_); }
  };
  struct CmdEndStream : public CmdSendSide {
   public:
    std::size_t GetHash() const { return typeid(this).hash_code(); }
    CmdEndStream() {}

   protected:
    int Call(std::shared_ptr<SendSide> sendSide) { sendSide->EndStream(); }
  };
  struct CmdResetStream : public CmdSendSide {
   public:
    std::size_t GetHash() const { return typeid(this).hash_code(); }
    CmdResetStream() {}

   protected:
    int Call(std::shared_ptr<SendSide> sendSide) { sendSide->ResetStream(); }
  };
  //                              O
  //                              +
  //                              | Create Stream
  //                              |
  // Send STREAM/                 v
  //      STREAM_DATA_BLOCKED +-------+  Send RESET_STREAM
  //             +----------->+ Send  +-----------------------+
  //             |            |       |                       |
  //             +------------+-------+                       |
  //                              |                           |
  //                              | Send STREAM+FIN           |
  //                              v                           v
  //                          +-------+  Send RESET_STREAM+-------+
  //                          | Data  +------------------>+ Reset |
  //                          | Sent  |                   | Sent  |
  //                          +-------+                   +-------+
  //                              |                           |
  //                              | Recv All ACKs             | Recv ACK
  //                              v                           v
  //                          +-------+                   +-------+
  //                          | Data  |                   | Reset |
  //                          | Recvd |                   | Recvd |
  //                          +-------+                   +-------+
  enum State : state_t {
    kSend,
    kDataSent,
    kResetSent,
    kDataRecvd,
    kResetRecvd,
  };
  enum TriggerType : trigger_t {
    kResetStream,   // Reset stream
    kSendFIN,       // Send Finish : Send -> Data Sent
    kRecvACKs,      // Recv All ACKs : Data Sent -> Data Recvd
    kRecvResetACK,  // Recv Reset ACK : Reset Sent -> Reset Recvd
  };
  explicit SendSide(Stream* const stream);
  void PushCommand(std::shared_ptr<CommandBase> cmd) {
    cmdQueue_->PushCmd(std::dynamic_pointer_cast<CmdSendSide>(cmd));
  }
#ifdef __MOST_TEST

 public:
#else

 private:
#endif
  struct SignalMask {
    enum Value { kBitEndStream, kBitResetStream };
  };
  Stream* const stream_;
  std::shared_ptr<AsynRoutine> routine_;
  std::shared_ptr<CommandQueue<CmdSendSide>> cmdQueue_;
  unsigned int flow_credit_ = 1500;
  // unsigned int mss_ = 512;
  std::stringstream send_buffer_;
  unsigned char signal_;
  inline std::streampos GetSendBufferLen() {
    return send_buffer_.tellp() - send_buffer_.tellg();
  }

  void SendData();
  void SendDataBlocked(std::streampos data_limit);

  int OnSend();
  int OnDataSent();
  int OnResetSent();
  int OnDataRecvd();
  int OnResetRecvd();

  void ConsumeCmd();
  void WriteData(std::shared_ptr<std::stringstream> gfl);
  void EndStream();
  void ResetStream();

  friend void* CoSendSide(void* arg);
};

class Stream {
 public:
  enum RecvSideState {
    kRecv,
    kSizeKnown,
    kDataRecvd,
    kResetRecvd,
    kDataRead,
    kResetRead
  };
  streamID_t id_;
  CommandExecutor* const conn_;
  Stream(CommandExecutor* const conn, streamID_t id, Initializer initer,
         Directional direct)
      : conn_(conn),
        id_(id),
        initer_(initer),
        direct_(direct),
        sendSide_(this),
        recvSide_(this) {}

#ifdef __MOSS_TEST

 public:
#else

 private:
#endif
  Initializer initer_;
  Directional direct_;
  SendSide sendSide_;
  SendSide recvSide_;
};

}  // namespace moss

#endif  // STREAM_H_
