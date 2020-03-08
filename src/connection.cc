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

#include "./connection.h"
#include "util/util.h"

namespace moss {

void* CoConnection(void* arg) {
  auto conn = reinterpret_cast<Connection*>(arg);
  while (true) {
    // TODO(Connection) : co func
  }
}

Connection::Connection(const ConnectionType& type) : type_(type) {
  routine_ = std::static_pointer_cast<AsynRoutine>(
      std::make_shared<Coroutine>(CoConnection, this));
  cmdQueue_ = std::static_pointer_cast<CommandQueue<CmdConnection>>(
      std::make_shared<CoCmdQueue<CmdConnection>>(routine_));
}

streamID_t Connection::NewID(const Initializer& initer,
                             const Directional& direct) {
  return ((nextIDPrefix_++) << 2) + (direct << 1) + initer;
}

std::shared_ptr<Stream> moss::Connection::CreateStream(Directional direct) {
  auto id = NewID(type_, direct);
  std::shared_ptr<Stream> stream =
      std::make_shared<Stream>(this, id, type_, direct);
  mapStreams_[id] = stream;
  return stream;
}

void Connection::PushCommand(std::shared_ptr<CommandBase> cmd) {
  auto _cmd = std::static_pointer_cast<CmdConnection>(cmd);
  cmdQueue_->PushCmdAndResume(std::static_pointer_cast<CmdConnection>(cmd));
}

void Connection::SendGFL(streamID_t stream_id,
                         std::shared_ptr<GenericFrameLayout> gfl) {
  mapStreamGFL_[stream_id].push(gfl);
}

void Connection::SendData(streamID_t stream_id, int send_count) {}

}  // namespace moss
