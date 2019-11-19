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

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <map>
#include <memory>
#include <queue>
#include "./command.h"
#include "./frame.h"
#include "./routine.h"
#include "./stream.h"

namespace moss {

struct CmdConnection : public CommandBase {
  int Execute(std::shared_ptr<void> arg) {
    auto connection = std::static_pointer_cast<Connection>(arg);
    return Call(connection);
  }

 protected:
  virtual int Call(std::shared_ptr<Connection> connection) = 0;
};
struct CmdSendGFL : public CmdConnection {
 public:
  std::size_t GetHash() const { return typeid(this).hash_code(); }
  CmdSendGFL(streamID_t stream_id, std::shared_ptr<GenericFrameLayout> gfl)
      : stream_id_(stream_id), gfl_(gfl) {}
  streamID_t stream_id_;
  std::shared_ptr<GenericFrameLayout> gfl_;
  int Call(std::shared_ptr<Connection> connection) {
    connection->SendGFL(stream_id_, gfl_);
  }
};

class Connection : public CommandExecutor,
                   public std::enable_shared_from_this<Connection> {
 public:
  explicit Connection(const ConnectionType& type) : type_(type) {}
  std::shared_ptr<Stream> CreateStream(Directional direct);
  void PushCommand(std::shared_ptr<CommandBase> cmd);

  void SendGFL(streamID_t stream_id, std::shared_ptr<GenericFrameLayout> gfl);

 private:
  streamID_t nextIDPrefix_ = 0;
  ConnectionType type_;
  std::shared_ptr<AsynRoutine> routine_;
  std::shared_ptr<CommandQueue<CmdConnection>> cmdQueue_;
  streamID_t NewID(const Initializer& initer, const Directional& direct);
  std::map<streamID_t, std::shared_ptr<Stream>> mapStreams_;
  std::map<streamID_t, std::queue<std::shared_ptr<GenericFrameLayout>>>
      mapStreamGFL_;

#ifdef __MOSS_TEST
  friend streamID_t __Test_NewID(std::shared_ptr<Connection> _this,
                                 const Initializer& initer,
                                 const Directional& direct) {
    return _this->NewID(initer, direct);
  }
#endif
};

}  // namespace moss

#endif  // CONNECTION_H_
