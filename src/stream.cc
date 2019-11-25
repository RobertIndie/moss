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
#include "./util/util.h"

namespace moss {

void* CoSendSide(void* arg) {
  auto sendside = reinterpret_cast<SendSide*>(arg);
  while (true) {
    sendside->ConsumeCmd();
    if (sendside->fsm_.Run() != -1) break;
  }
}

SendSide::SendSide(Stream* const stream) : stream_(stream) {
  fsm_.When(TriggerType::kSendFIN,
            FSM::transition_t(State::kSend, State::kDataSent));
  fsm_.When(TriggerType::kResetStream,
            FSM::transition_t(State::kSend, State::kResetSent));
  fsm_.When(TriggerType::kResetStream,
            FSM::transition_t(State::kDataSent, State::kResetSent));
  fsm_.When(TriggerType::kRecvACKs,
            FSM::transition_t(State::kDataSent, State::kDataRecvd));
  fsm_.When(TriggerType::kRecvResetACK,
            FSM::transition_t(State::kResetSent, State::kResetRecvd));
  fsm_.On(State::kSend, std::bind(&SendSide::OnSend, this));
  fsm_.On(State::kDataSent, std::bind(&SendSide::OnDataSent, this));
  fsm_.On(State::kResetSent, std::bind(&SendSide::OnResetSent, this));
  fsm_.On(State::kDataRecvd, std::bind(&SendSide::OnDataRecvd, this));
  fsm_.On(State::kResetRecvd, std::bind(&SendSide::OnResetRecvd, this));
  fsm_.SetState(State::kSend);
  routine_ = std::shared_ptr<AsynRoutine>(
      reinterpret_cast<AsynRoutine*>(new Coroutine(CoSendSide, this)));
  cmdQueue_ = std::shared_ptr<CommandQueue<CmdSendSide>>(
      reinterpret_cast<CommandQueue<CmdSendSide>*>(
          new CoCmdQueue<CmdSendSide>(routine_)));
  routine_->Resume();  // Create Stream (Sending)
}

void SendSide::SendData() {
  // TODO(connection): send data
}

void SendSide::SendDataBlocked(std::streampos data_limit) {
  // TODO(gfl-fraeme): use frame conversion
  std::shared_ptr<GenericFrameLayout> gfl(new GenericFrameLayout);
  gfl->frame_type = FrameType::kStreamDataBlocked;
  std::shared_ptr<CmdSendGFL> cmd(new CmdSendGFL(stream_->id_, gfl));
  stream_->conn_->PushCommand(std::static_pointer_cast<CommandBase>(cmd));
}

int SendSide::OnSend() {
  if (flow_credit_ <= send_buffer_.tellg()) {
    SendDataBlocked(flow_credit_);
  }
  if (GetSendBufferLen() > 0) {
    SendData();
  }
  return 0;
}

int SendSide::OnDataSent() { return 0; }

int SendSide::OnResetSent() { return 0; }

int SendSide::OnDataRecvd() { return 0; }

int SendSide::OnResetRecvd() { return 0; }

void SendSide::ConsumeCmd() { auto cmd = cmdQueue_->WaitAndExecuteCmds(this); }

void SendSide::WriteData(std::shared_ptr<std::stringstream> data) {
  send_buffer_ << data->str();
}

void SendSide::EndStream() { AddSignal(signal_, SignalMask::kBitEndStream); }

void SendSide::ResetStream() {
  AddSignal(signal_, SignalMask::kBitResetStream);
}

}  // namespace moss
