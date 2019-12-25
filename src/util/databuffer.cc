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

#define READ_LOCK(lock) pthread_rwlock_rdlock(&lock);
#define WRITE_LOCK(lock) pthread_rwlock_wrlock(&lock);
#define DEFER_UNLOCK(lock) DEFER(pthread_rwlock_unlock(&lock);)

DPTR& DPTR::operator+=(Index_t offset) {
  ptr_ = Move(ptr_, offset);
  return *this;
}

bool DPTR::operator<(const DPTR& rhs) {
  auto offset = buffer_->cap_size_ - buffer_->writer_pos_ - 1;
  return Move(ptr_, offset) < Move(rhs.ptr_, offset);
}

bool DPTR::operator>(const DPTR& rhs) { return rhs < *this; }

Index_t DPTR::Move(Index_t ptr, Index_t offset) {
  auto result = (static_cast<Index_t>(ptr) + offset) % buffer_->cap_size_;
  if (result < 0) {
    result += buffer_->cap_size_;
  }
  return result;
}

DataBuffer::DataBuffer(size_t init_size, bool fixed_size)
    : block_size(init_size), fixed_size_(fixed_size) {
  block_ = std::make_shared<DataBlock>(init_size);
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

int DataBuffer::Read(std::shared_ptr<DataReader> reader, const int count,
                     char* data) {
  READ_LOCK(lock_);
  DEFER_UNLOCK(lock_)
  // check constraint, set end_ptr
  uint64_t end_ptr;
  if (reader->constraint_ != nullptr) {
    end_ptr = reader->constraint_->ptr_;
  } else {
    end_ptr = writer_pos_;
  }
  if (MovePtr(end_ptr, -reader->ptr_) > count) {
    end_ptr = MovePtr(reader->ptr_, count);
  }
  // get data
  if (data == nullptr) {
    reader->ptr_ = end_ptr;
  } else {
    // TODO(GetData)
  }
  auto read_count = MovePtr(end_ptr, -reader->ptr_);
  reader->ptr_ = end_ptr;
  // resize
  if (auto s = blocks_.size() >= 2) {
    auto last = blocks_[s - 1];
    float thresold = static_cast<float>(last->len_) * 3 / 4;
  }
  return read_count;
}

int DataBuffer::Writer(const int count, const char* data) {
  WRITE_LOCK(lock_);
  DEFER_UNLOCK(lock_)
}
