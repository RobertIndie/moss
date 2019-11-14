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

#include "./util/util.h"

stShareStack_t* share_stack;

stCoRoutine_t* CreateCoroutine(pfn_co_routine_t pfn, void* arg) {
  if (share_stack == nullptr) share_stack = co_alloc_sharestack(1, 1024 * 128);
  stCoRoutineAttr_t attr;
  attr.stack_size = 0;
  attr.share_stack = &*share_stack;
  stCoRoutine_t* co_p;
  co_create(&co_p, &attr, pfn, arg);
  return co_p;
}
