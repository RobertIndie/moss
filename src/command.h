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

#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <type_traits>
#include <vector>
#include "./routine.h"
#include "./util/util.h"

namespace moss {
struct CommandBase {
 public:
  virtual std::size_t GetHash() const { return typeid(this).hash_code(); }

 protected:
  virtual int Execute(void*const arg) = 0;
  template <typename CmdType>
  friend class CommandQueue;
};

template <typename CmdType>
class CommandQueue {
  static_assert(std::is_base_of<CommandBase, CmdType>::value,
                "class CmdType must inherit from class CommandBase");

 public:
  virtual void PushCmd(std::shared_ptr<CmdType> cmd) = 0;
  virtual std::shared_ptr<CmdType> WaitCmd() = 0;
  virtual int WaitAndExecuteCmds(void*const arg) = 0;
};

template <typename CmdType>
class CoCmdQueue : public CommandQueue<CmdType> {
 public:
  explicit CoCmdQueue(std::shared_ptr<AsynRoutine> co) : co_(co) {}
  void PushCmd(std::shared_ptr<CmdType> command) {
    command_queue_.push(command);
    co_->Resume();
  }
  std::shared_ptr<CmdType> WaitCmd() {
    while (command_queue_.size() == 0) {
      co_->Suspend();
    }
    auto cmd = command_queue_.front();
    command_queue_.pop();
    return cmd;
  }
  int WaitAndExecuteCmds(void*const arg) {
    co_->Suspend();
    if (command_queue_.size() == 0) return 0;  // The timer is up
    int cmd_count = command_queue_.size();
    for (int i = 0; i < cmd_count; ++i) {
      auto cmd = command_queue_.front();
      cmd->Execute(arg);
    }
    return cmd_count;
  }

 protected:
  std::shared_ptr<AsynRoutine> co_;
  std::queue<std::shared_ptr<CmdType> > command_queue_;
};

class CommandExecutor {
 public:
  virtual void PushCommand(std::shared_ptr<CommandBase> cmd) = 0;
};

}  // namespace moss

#endif  // COMMAND_H_
