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

#ifndef COMMAND_H_
#define COMMAND_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <vector>
#include "./routine.h"
#include "./util/util.h"

namespace moss {
// Command ID Type
typedef uint32_t cid_t;
struct CommandBase {
 public:
  explicit CommandBase(cid_t type) : type_(type) {}
  cid_t GetType() { return type_; }

 protected:
  cid_t type_ = 0;
};

class CommandQueue {
 public:
  virtual void PushCmd(std::shared_ptr<CommandBase> cmd) = 0;
  virtual std::shared_ptr<CommandBase> WaitCmd() = 0;
};

class CoCmdQueue : CommandQueue {
 public:
  explicit CoCmdQueue(std::shared_ptr<AsynRoutine> co) : co_(co) {}
  void PushCmd(std::shared_ptr<CommandBase> command) {
    command_queue_.push(command);
    co_->Resume();
  }
  std::shared_ptr<CommandBase> WaitCmd() {
    while (command_queue_.size() == 0) {
      co_->Suspend();
    }
    auto cmd = command_queue_.front();
    command_queue_.pop();
    return cmd;
  }

 protected:
  std::shared_ptr<AsynRoutine> co_;
  std::queue<std::shared_ptr<CommandBase> > command_queue_;
};

}  // namespace moss

#endif  // COMMAND_H_
