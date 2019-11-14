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
#include "./co_routine.h"
#include "./command.h"
#include "./fsm/fsm.h"

namespace moss {

enum Directional { kBidirectional = 0, kUnidirectional = 1 };
enum ConnectionType { kClient = 0, kServer = 1 };
typedef ConnectionType Initializer;
typedef uint64_t streamID_t;

struct CommandSide : public CommandBase {};
struct CommandSendSide : public CommandSide {};

class StreamSide {
 public:
  StreamSide() : fsm_(0) {}

 protected:
  FSM fsm_;
};

class SendSide : public StreamSide {
 public:
  enum State {
    kReady,
    kSend,
    kDataSent,
    kResetSent,
    kDataRecvd,
    kResetRecvd,
  };
  enum TriggerType {
    kResetStream,        // Send RESET_STREAM
    kStream,             // Send STREAM
    kStreamDataBlocked,  // Send STREAM_DATA_BLOCKED
    kCreateBiStream,     // Peer Creates Bidirectional Steram
    kStreamFin,          // Send STREAM + FIN
    kRecvAllACKs,        // Recv All ACKs
    kRecvAck,            // Recv ACK
  };
  SendSide();
  ~SendSide();
  void StartCoroutine();

 private:
  std::queue<CommandSendSide> command_queue_;
  stCoRoutine_t* co_;
  int OnReady();
  int OnSend();
  int OnDataSent();
  int OnResetSent();
  int OnDataRecvd();
  int OnResetRecvd();
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

  Stream(streamID_t id, Initializer initer, Directional direct)
      : id_(id), initer_(initer), direct_(direct), sendSide_(), recvSide_(){};

#ifdef __MOSS_TEST
 public:
#else
 private:
#endif
  streamID_t id_;
  Initializer initer_;
  Directional direct_;
  SendSide sendSide_;
  SendSide recvSide_;
};

}  // namespace moss

#endif  // STREAM_H_
