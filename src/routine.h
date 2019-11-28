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

#include "./co_routine.h"
#ifndef ROUTINE_H_
#define ROUTINE_H_

namespace moss {

class AsynRoutine {
 public:
  virtual void Resume() = 0;
  virtual void Suspend() = 0;
};

class Coroutine : public AsynRoutine {
 public:
  static stShareStack_t *share_stack;
  Coroutine() {}
  Coroutine(pfn_co_routine_t pfn, void *arg) { Init(pfn, arg); }
  void Init(pfn_co_routine_t pfn, void *arg) {
    if (share_stack == nullptr)
      share_stack = co_alloc_sharestack(1, 1024 * 128);
    stCoRoutineAttr_t attr;
    attr.stack_size = 0;
    attr.share_stack = &*share_stack;
    co_create(&co_, &attr, pfn, arg);
  }
  void Resume() { co_resume(co_); }
  void Suspend() { co_yield_ct(); }
  ~Coroutine() { co_release(co_); }

 private:
  stCoRoutine_t *co_;
};

}  // namespace moss

#endif  // ROUTINE_H_
