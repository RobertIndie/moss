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

#ifndef UTIL_FSM_H_
#define UTIL_FSM_H_
#include <functional>
#include <map>
#include <tuple>
#include <utility>
#include <vector>

typedef int state_t;
typedef int trigger_t;

class FSM {
 public:
  explicit FSM(state_t init_state) : state_(init_state) {}
  typedef std::function<int()> on_func_t;
  typedef std::pair<state_t, state_t> transition_t;
  void SetState(state_t state) { state_ = state; }
  state_t GetState() const { return state_; }
  void When(const trigger_t& trigger, const transition_t& transition);
  void On(const state_t& state, const std::function<int()>& on_func);
  void Trigger(const trigger_t& trigger);
  int Run() const;

 private:
  state_t state_;
  std::map<trigger_t, std::map<state_t, state_t>> triggers;
  std::map<state_t, on_func_t> state_events;
};

#endif  // UTIL_FSM_H_
