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
typedef uint32_t ID_t;
class DPTR {
 public:
  friend class DataBuffer;
  ID_t ptrID = 0;
  DPTR operator+(DPTR& dptr);

 private:
  uint64_t ptr_ = 0;
  DataBuffer* buffer_;
};

class DataReader : public DPTR {
 public:
  friend class DataBuffer;
  int Read(const int count = -1, char* data = nullptr);
  const DataReader* constraint_ = nullptr;
};

class DataBuffer {
 public:
  friend class DataReader;
  explicit DataBuffer(size_t init_size = 8, bool fixed_size = false);
  ~DataBuffer();
  std::shared_ptr<DataReader> NewReader(ID_t constraintPtr = -1);

 private:
  struct DataBlock {
    char* const buffer_;
    const size_t len_;
    explicit DataBlock(int len) : len_(len), buffer_(new char[len_]) {}
    ~DataBlock() { delete[] buffer_; }
  };
  std::vector<DataBlock*> blocks_;
  uint64_t cap_size_ = 0;
  uint64_t data_size_ = 0;
  std::vector<std::shared_ptr<DataReader>> readers_;
  bool fixed_size_;
  const size_t block_size;  // data size in the first data block
  pthread_rwlock_t lock_;
  uint64_t writer_pos_ = 0;
  uint64_t MovePtr(uint64_t ptr, int64_t offset);
  int Read(std::shared_ptr<DataReader> reader, const int count, char* data);
  int Writer(const int count, const char* data);
};

#endif  // UTIL_DATABUFFER_H_
