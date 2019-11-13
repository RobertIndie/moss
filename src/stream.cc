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

#include "./stream.h"

namespace moss {

SendSide::SendSide() {
  fsm.When(TriggerType::kStream,
           FSM::transition_t(State::kReady, State::kSend));
  fsm.When(TriggerType::kStreamDataBlocked,
           FSM::transition_t(State::kReady, State::kSend));
  fsm.When(TriggerType::kCreateBiStream,
           FSM::transition_t(State::kReady, State::kSend));
  fsm.When(TriggerType::kStreamFin,
           FSM::transition_t(State::kSend, State::kDataSent));
  fsm.When(TriggerType::kResetStream,
           FSM::transition_t(State::kReady, State::kResetSent));
  fsm.When(TriggerType::kResetStream,
           FSM::transition_t(State::kSend, State::kResetSent));
  fsm.When(TriggerType::kResetStream,
           FSM::transition_t(State::kDataSent, State::kResetSent));
  fsm.When(TriggerType::kRecvAllACKs,
           FSM::transition_t(State::kDataSent, State::kDataRecvd));
  fsm.When(TriggerType::kRecvAck,
           FSM::transition_t(State::kResetSent, State::kResetRecvd));
  fsm.On(State::kReady, std::bind(&SendSide::OnReady, *this));
  fsm.On(State::kSend, std::bind(&SendSide::OnSend, *this));
  fsm.On(State::kDataSent, std::bind(&SendSide::OnDataSent, *this));
  fsm.On(State::kResetSent, std::bind(&SendSide::OnResetSent, *this));
  fsm.On(State::kDataRecvd, std::bind(&SendSide::OnDataRecvd, *this));
  fsm.On(State::kResetRecvd, std::bind(&SendSide::OnResetRecvd, *this));
}

int SendSide::OnReady() {

}

}  // namespace moss
