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
class DataBuffer;
typedef int64_t Index_t;
class DPTR {
 public:
  friend class DataBuffer;
  friend class DataReader;
  DPTR& operator+=(Index_t offset);
  bool operator<(const DPTR& rhs) const;
  bool operator>(const DPTR& rhs) const;
  bool operator<=(const DPTR& rhs) const;
  bool operator>=(const DPTR& rhs) const;
  bool operator==(const DPTR& rhs) const;
  bool operator!=(const DPTR& rhs) const;

#ifdef __MOSS_TEST
 public:
#else
 private:
#endif
  explicit DPTR(DataBuffer* const buffer)
      : buffer_(buffer) {}  // 私有构造函数使得不能直接进行实例化
  Index_t ptr_ = 0;
  DataBuffer* const buffer_;
  static Index_t Move(const DataBuffer* const buffer, Index_t ptr,
                      Index_t offset);
};

class DataReader : public DPTR {
 public:
  friend class DataBuffer;
  DataReader(DataBuffer* const buffer, const DataReader* const constraint)
      : DPTR(buffer), constraint_(constraint) {}
  int Read(const int count, char* data = nullptr);

 private:
  const DataReader* const constraint_;
};

class DataBuffer {
 public:
  friend class DataReader;
  friend class DPTR;
  explicit DataBuffer(size_t init_size = 8, bool fixed_size = false);
  ~DataBuffer();
  std::shared_ptr<DataReader> NewReader(
      const DataReader* const constraintReader = nullptr);

#ifdef __MOSS_TEST
 public:
#else
 private:
#endif
  struct DataBlock {
    char* const buffer_;
    const size_t len_;
    explicit DataBlock(int len) : len_(len), buffer_(new char[len_]) {}
    ~DataBlock() { delete[] buffer_; }
  };
  std::shared_ptr<DataBlock> block_;
  uint64_t cap_size_ = 0;
  uint64_t data_size_ = 0;
  std::vector<std::shared_ptr<DataReader>> readers_;
  bool fixed_size_;
  const size_t block_size;  // data size in the first data block
  pthread_rwlock_t lock_;
  uint64_t writer_pos_ = 0;  // 准备写入的位置
  uint64_t MovePtr(uint64_t ptr, int64_t offset);
  int Read(DataReader* const reader, const int count,
           char* data = nullptr);  // 将自动申请内存
  int Write(const int count, const char* data);
};

#endif  // UTIL_DATABUFFER_H_
