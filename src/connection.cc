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

streamID_t Connection::NewID(const Initializer& initer,
                             const Directional& direct) {
  return ((nextIDPrefix_++) << 2) + (direct << 1) + initer;
}

std::shared_ptr<Stream> moss::Connection::CreateStream(Directional direct) {
  auto id = NewID(type_, direct);
  std::shared_ptr<Stream> stream(new Stream(id, type_, direct));
  mapStreams_[id] = stream;
  PLOG(INFO) << "[Connection]Create Stream: " << LOG_VALUE(id)
             << LOG_VALUE(type_) << LOG_VALUE(direct);
  return stream;
}

}  // namespace moss
