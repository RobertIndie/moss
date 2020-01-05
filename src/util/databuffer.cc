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
#include <math.h>
#include <algorithm>
#include "util/util.h"

#define READ_LOCK(lock) pthread_rwlock_rdlock(&lock);
#define WRITE_LOCK(lock) pthread_rwlock_wrlock(&lock);
#define DEFER_UNLOCK(lock) DEFER(pthread_rwlock_unlock(&lock);)

DPTR& DPTR::operator+=(Index_t offset) {
  ptr_ = Move(buffer_, ptr_, offset);
  return *this;
}

bool DPTR::operator<(const DPTR& rhs) const {
  auto offset = buffer_->cap_size_ - buffer_->writer_pos_ - 1;
  return Move(buffer_, ptr_, offset) < Move(buffer_, rhs.ptr_, offset);
}

bool DPTR::operator>(const DPTR& rhs) const { return rhs < *this; }

bool DPTR::operator<=(const DPTR& rhs) const { return !(rhs > *this); }

bool DPTR::operator>=(const DPTR& rhs) const { return !(rhs < *this); }

bool DPTR::operator==(const DPTR& rhs) const { return rhs.ptr_ == this->ptr_; }

bool DPTR::operator!=(const DPTR& rhs) const {
  return !(rhs.ptr_ == this->ptr_);
}

Index_t DPTR::Move(const DataBuffer* const buffer, Index_t ptr,
                   Index_t offset) {
  auto result = (static_cast<Index_t>(ptr) + offset) % buffer->cap_size_;
  if (result < 0) {
    result += buffer->cap_size_;
  }
  return result;
}

int DataReader::Read(const int count, char* data) {
  return buffer_->Read(this, count, data);
}

DataBuffer::DataBuffer(size_t init_size, bool fixed_size)
    : block_size(init_size), fixed_size_(fixed_size) {
  block_ = std::make_shared<DataBlock>(init_size);
  cap_size_ = init_size;
  pthread_rwlock_init(&lock_, nullptr);
}

DataBuffer::~DataBuffer() {
  // for (auto iter = blocks_.begin(); iter != blocks_.end(); ++iter) {
  //   auto p = *iter;
  //   if (p != nullptr) delete p;
  // }
}

std::shared_ptr<DataReader> DataBuffer::NewReader(
    const DataReader* const constraintReader) {
  WRITE_LOCK(lock_);
  DEFER_UNLOCK(lock_)
  auto reader = std::make_shared<DataReader>(this, constraintReader);
  readers_.push_back(reader);
  return reader;
}

uint64_t DataBuffer::MovePtr(uint64_t ptr, int64_t offset) {
  auto result = (static_cast<int64_t>(ptr) + offset) % cap_size_;
  if (result < 0) {
    result += cap_size_;
  }
  return result;
}

void DataBuffer::Resize(std::shared_ptr<DataReader> min, Index_t new_cap_size) {
  // auto new_block = std::make_shared<DataBlock>(new_cap_size);
  std::shared_ptr<DataBlock> new_block(new DataBlock(new_cap_size));
  auto start_pos = min->ptr_;
  if (writer_pos_ >= start_pos) {
    memcpy(new_block->buffer_, block_->buffer_ + start_pos,
           writer_pos_ - start_pos);
  } else {
    memcpy(new_block->buffer_, block_->buffer_ + start_pos,
           cap_size_ - start_pos);
    memcpy(new_block->buffer_, block_->buffer_, writer_pos_);
  }
  for (auto iter = readers_.begin(); iter != readers_.end(); iter++) {
    DPTR::Move(this, (**iter).ptr_, -start_pos);
  }
  writer_pos_ = DPTR::Move(this, writer_pos_, -start_pos);
  cap_size_ = new_cap_size;
  block_ = new_block;
}

int DataBuffer::Read(DataReader* const reader, const int count, char* data) {
  READ_LOCK(lock_);
  DEFER_UNLOCK(lock_)
  // check constraint, set end_ptr
  uint64_t end_ptr;
  if (reader->constraint_ != nullptr) {
    end_ptr = reader->constraint_->ptr_;
  } else {
    end_ptr = writer_pos_;
  }
  auto offset = cap_size_ - writer_pos_ - 1;
  auto maxCount = DPTR::Move(this, end_ptr, offset) -
                  DPTR::Move(this, reader->ptr_, offset);
  if (count != -1 && maxCount > count) {
    end_ptr = MovePtr(reader->ptr_, count);
  }
  // get data
  Index_t read_count;
  auto diff = end_ptr - reader->ptr_;
  if (diff >= 0) {
    // out_data = new char[diff];
    if (data != nullptr) memcpy(data, block_->buffer_ + reader->ptr_, diff);
    read_count = diff;
  } else {
    diff += cap_size_;
    if (data != nullptr) {
      // out_data = new char[diff];
      memcpy(data, block_->buffer_ + reader->ptr_, cap_size_ - reader->ptr_);
      memcpy(data + cap_size_ - reader->ptr_, block_->buffer_, end_ptr);
    }
    read_count = cap_size_ - reader->ptr_ + end_ptr;
  }
  reader->ptr_ = end_ptr;
  auto min = *std::max_element(readers_.begin(), readers_.end());
  data_size_ = DPTR::Move(this, writer_pos_, offset) -
               DPTR::Move(this, min->ptr_, offset);
// resize
#define SCHMIDT_COEFFICIENT (3.0 / 8.0)  // 范围为0-1
  auto new_cap_size = cap_size_;
  while (
      data_size_ <
          new_cap_size *
              SCHMIDT_COEFFICIENT &&  // 需要使用小于号，当SCHMIDT_COEFFICIENT
                                      // 为1时，则可预留空间
      new_cap_size > block_size) {  // 施密特触发降低resize灵敏度
    new_cap_size /= 2;
  }
  if (new_cap_size != cap_size_) Resize(min, new_cap_size);
  return read_count;
}

int DataBuffer::Write(const int count, const char* data) {
  WRITE_LOCK(lock_);
  DEFER_UNLOCK(lock_)
  auto new_data_size = data_size_ + count;
  // resize
  if (new_data_size >= cap_size_) {
    auto offset = cap_size_ - writer_pos_ - 1;
    if (readers_.size() == 0) {
      DLOG(ERROR) << "No readers.";
      throw "No readers.";
    }
    auto min = *std::max_element(readers_.begin(), readers_.end());
    auto current_cap_size = cap_size_;
    while (current_cap_size <= new_data_size) {  // 需要预留一字节空间
      current_cap_size *= 2;
    }
    Resize(min, current_cap_size);
  }
  auto end_ptr = DPTR::Move(this, writer_pos_, count);
  auto diff = end_ptr - writer_pos_;
  if (diff >= 0) {
    memcpy(block_->buffer_ + writer_pos_, data, diff);
  } else {
    memcpy(block_->buffer_ + writer_pos_, data, cap_size_ - writer_pos_);
    memcpy(block_->buffer_, data + cap_size_ - writer_pos_, end_ptr);
  }
  writer_pos_ = end_ptr;
  data_size_ = new_data_size;
  return count;
}
