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

namespace moss {
// Command ID Type
typedef uint32_t cid_t;
struct CommandBase {
 public:
  CommandBase(cid_t type) : type_(type) {}
  cid_t GetType() { return type_; }

 protected:
  cid_t type_ = 0;
};

class CommandExecutor {
 public:
  virtual void PushCommand(std::shared_ptr<CommandBase> command) {
    command_queue_.push(command);
  }

 protected:
  std::queue<std::shared_ptr<CommandBase> > command_queue_;
  void RegisterCommand(const cid_t& command_id, std::function<int()> func);

  virtual std::shared_ptr<CommandBase> PopCommand() {
    auto cmd = command_queue_.front();
    command_queue_.pop();
    return cmd;
  }
};

}  // namespace moss

#endif  // COMMAND_H_
