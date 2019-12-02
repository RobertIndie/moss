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
#include "./interfaces.h"
#include "./routine.h"
#include "./stream.h"

namespace moss {

class Connection : virtual public IConnection, public CommandExecutor {
 public:
  explicit Connection(const ConnectionType& type);
  std::shared_ptr<Stream> CreateStream(Directional direct);

#pragma region CommandExecutor
  void PushCommand(std::shared_ptr<CommandBase> cmd);
#pragma endregion

#pragma region IConnection
  void SendGFL(streamID_t stream_id, std::shared_ptr<GenericFrameLayout> gfl);
  void SendData(streamID_t stream_id, std::stringstream* buffer, int data_len);
#pragma endregion

#ifdef __MOSS_TEST

 public:
#else

 private:
#endif
  streamID_t nextIDPrefix_ = 0;
  ConnectionType type_;
  std::shared_ptr<AsynRoutine> routine_;
  std::shared_ptr<CommandQueue<CmdConnection>> cmdQueue_;
  streamID_t NewID(const Initializer& initer, const Directional& direct);
  std::map<streamID_t, std::shared_ptr<Stream>> mapStreams_;
  std::map<streamID_t, std::queue<std::shared_ptr<GenericFrameLayout>>>
      mapStreamGFL_;

  friend void* CoConnection(void* arg);

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
