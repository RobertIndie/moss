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

#include "util/fsm.h"
#include <sstream>
#include "util/util.h"

void FSM::When(const trigger_t& trigger, const transition_t& transition) {
  auto trigger_map = triggers[trigger];
  auto trans_ele = trigger_map.find(transition.first);
  if (trans_ele != trigger_map.end()) {
    PLOG(FATAL) << "[FSM]State transition conflict. ";
  }
  triggers[trigger].insert(transition);
}

void FSM::On(const state_t& state, const std::function<int()>& on_func) {
  state_events[state] = on_func;
}

void FSM::Trigger(const trigger_t& trigger) {
  auto trigger_ele = triggers.find(trigger);
  if (trigger_ele == triggers.end()) {
    PLOG(FATAL) << "[FSM]Cannot find trigger:" << trigger;
  }
  auto trigger_map = trigger_ele->second;
  auto trans_ele = trigger_map.find(state_);
  if (trans_ele == trigger_map.end()) {
    PLOG(FATAL) << "[FSM]Cannot find transition. " << LOG_VALUE(state_)
                << LOG_VALUE(trigger);
  }
  state_ = trans_ele->second;
}

int FSM::Run() const {
  auto ele = state_events.find(state_);
  if (ele != state_events.end()) {
    return ele->second();
  }
}
