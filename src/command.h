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
#include <memory>
#include <vector>

namespace moss {

struct CommandBase {
 protected:
  uint32_t type = 0;
  std::shared_ptr<void> arg;
};

}  // namespace moss

#endif  // COMMAND_H_
