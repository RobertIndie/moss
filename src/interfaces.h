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

#ifndef INTERFACES_H_
#define INTERFACES_H_

#include <memory>
#include "./frame.h"

namespace moss {

enum Directional { kBidirectional = 0, kUnidirectional = 1 };
enum ConnectionType { kClient = 0, kServer = 1 };
typedef ConnectionType Initializer;
typedef uint64_t streamID_t;

class IConnection {
 public:
  virtual void SendGFL(streamID_t stream_id,
                       std::shared_ptr<GenericFrameLayout> gfl) = 0;
};

#pragma region Commands
struct CmdConnection : public CommandBase {
  int Execute(void*const arg) {
    auto connection = static_cast<IConnection*const>(arg);
    return Call(connection);
  }

 protected:
  virtual int Call(IConnection*const connection) = 0;
};

struct CmdSendGFL : public CmdConnection {
 public:
  std::size_t GetHash() const { return typeid(this).hash_code(); }
  CmdSendGFL(streamID_t stream_id, std::shared_ptr<GenericFrameLayout> gfl)
      : stream_id_(stream_id), gfl_(gfl) {}
  streamID_t stream_id_;
  std::shared_ptr<GenericFrameLayout> gfl_;
  int Call(IConnection*const connection) {
    connection->SendGFL(stream_id_, gfl_);
  }
};
#pragma endregion

}  // namespace moss

#endif  // INTERFACES_H_
