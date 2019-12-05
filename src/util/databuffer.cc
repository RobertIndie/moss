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

#include "util/databuffer.h"
#include "util/util.h"

DataBuffer::DataBuffer(size_t init_size, bool fixed_size)
    : block_size(init_size), fixed_size_(fixed_size) {
  DataBlock* block = new DataBlock(init_size);
  blocks_.push_back(block);
}

DataBuffer::~DataBuffer() {
  for (auto iter = blocks_.begin(); iter != blocks_.end(); ++iter) {
    auto p = *iter;
    if (p != nullptr) delete p;
  }
}

std::shared_ptr<DataReader> DataBuffer::NewReader(ID_t constraintReader) {
  pthread_rwlock_rdlock(&lock_);
  DEFER(pthread_rwlock_unlock(&lock_);)
  auto reader = std::make_shared<DataReader>();
  if (constraintReader != -1) {
    reader->constraint_ =
        readers_[constraintReader]
            .get();  // when other reader closed, it may be null
  }
  reader->ptrID = readers_.size() - 1;
  readers_.push_back(reader);
  return reader;
}

std::shared_ptr<DataWriter> DataBuffer::NewWriter() {
  pthread_rwlock_rdlock(&lock_);
  DEFER(pthread_rwlock_unlock(&lock_);)
  auto writer = std::make_shared<DataWriter>();
  writer->ptrID = writers_.size() - 1;
  writers_.push_back(writer);
  return writer;
}
