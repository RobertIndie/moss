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

#ifndef UTIL_DATABUFFER_H_
#define UTIL_DATABUFFER_H_

#include <pthread.h>
#include <memory>
#include <vector>

class DPTR {
 public:
  DataBuffer::ID_t ptrID = 0;
  uint64_t ptr_ = 0;
  const DPTR* constraint_ = nullptr;
};

class DataReader : public DPTR {
 public:
  void Read(int count = -1, char* data = nullptr);
};

class DataWriter : public DPTR {
 public:
  void Write(int count, char* data);
};

class DataBuffer {
 public:
  typedef uint32_t ID_t;
  explicit DataBuffer(size_t init_size = 8, bool fixed_size = false);
  ~DataBuffer();
  std::shared_ptr<DataReader> NewReader(ID_t constraintPtr = -1);
  std::shared_ptr<DataWriter> NewWriter();
  friend struct Reader;

 private:
  struct DataBlock {
    char* const buffer_;
    const size_t len_;
    explicit DataBlock(int len) : len_(len), buffer_(new char[len_]) {}
    ~DataBlock() { delete[] buffer_; }
  };
  std::vector<DataBlock*> blocks_;
  std::vector<std::shared_ptr<DataReader>> readers_;
  std::vector<std::shared_ptr<DataWriter>> writers_;
  bool fixed_size_;
  const size_t block_size;
  pthread_rwlock_t lock_;
};

#endif  // UTIL_DATABUFFER_H_
