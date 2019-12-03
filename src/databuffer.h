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

#ifndef DATABUFFER_H_
#define DATABUFFER_H_

#include <memory>
#include <vector>

class DataBuffer {
 public:
  typedef uint32_t ID_t;
  explicit DataBuffer(size_t init_size = 8, bool fixed_size = false)
      : block_size(init_size), fixed_size_(fixed_size) {
    DataBlock* block = new DataBlock(init_size);
    blocks_.push_back(block);
  }
  ~DataBuffer() {
    for (auto iter = blocks_.begin(); iter != blocks_.end(); ++iter) {
      auto p = *iter;
      if (p != nullptr) delete p;
    }
  }
  ID_t NewReader(ID_t constraintReader = -1) {
    DPTR reader;
    reader.ptr_ = 0;
    if (constraintReader != -1) {
      reader.constraint_ = &readers_[constraintReader];
    }
    readers_.push_back(reader);
    return readers_.size() - 1;
  }

 private:
  struct DPTR {
    ID_t block_id = 0;
    uint64_t ptr_;
    const DPTR* constraint_;
  };
  struct DataBlock {
    char* const buffer_;
    const size_t len_;
    explicit DataBlock(int len) : len_(len), buffer_(new char[len_]) {}
    ~DataBlock() { delete[] buffer_; }
  };
  std::vector<DataBlock*> blocks_;
  std::vector<DPTR> readers_;
  std::vector<DPTR> writers_;
  bool fixed_size_;
  const size_t block_size;
};

#endif  // DATABUFFER_H_
