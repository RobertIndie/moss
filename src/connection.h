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
#include "./stream.h"

namespace moss {

class Connection {
 public:
  explicit Connection(const ConnectionType& type) : type_(type) {}
  std::shared_ptr<Stream> CreateStream(Directional direct);

 private:
  streamID_t nextIDPrefix_ = 0;
  ConnectionType type_;
  streamID_t NewID(const Initializer& initer, const Directional& direct);
  std::map<streamID_t, std::shared_ptr<Stream> > mapStreams_;
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
