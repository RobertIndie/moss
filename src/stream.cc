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
  routine_->Resume();  // Init
}

void SendSide::SendData() {
  auto cmd = std::make_shared<CmdSendData>(stream_->id_, &send_buffer_,
                                           GetSendBufferLen());
  stream_->conn_->PushCommand(std::static_pointer_cast<CommandBase>(cmd));
}

void SendSide::SendDataBlocked(std::streampos data_limit) {
  auto gfl = std::make_shared<GenericFrameLayout>();
  auto frame = std::make_shared<FrameStreamDataBlocked>();
  frame->stream_data_limit = data_limit;
  frame->stream_id = stream_->id_;
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
  if (CheckSignal(signal_, SignalMask::kBitEndStream)) {
    ClearSignal(signal_, SignalMask::kBitEndStream);
    fsm_.Trigger(TriggerType::kSendFIN);
  }
  if (CheckSignal(signal_, SignalMask::kBitResetStream)) {
    ClearSignal(signal_, SignalMask::kBitResetStream);
    fsm_.Trigger(TriggerType::kResetStream);
  }
  return 0;
}

int SendSide::OnDataSent() {
  if (CheckSignal(signal_, SignalMask::kBitResetStream)) {
    ClearSignal(signal_, SignalMask::kBitResetStream);
    fsm_.Trigger(TriggerType::kResetStream);
  }
  return 0;
}

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

void Stream::WriteData(const char* data, int data_len) {
  auto data_ss = std::make_shared<std::stringstream>();
  data_ss->write(data, data_len);
  sendSide_.PushCommand(std::dynamic_pointer_cast<CommandBase>(
      std::make_shared<SendSide::CmdWriteData>(data_ss)));
}

void Stream::EndStream() {
  sendSide_.PushCommand(std::dynamic_pointer_cast<CommandBase>(
      std::make_shared<SendSide::CmdEndStream>()));
}

void Stream::ResetStream() {
  sendSide_.PushCommand(std::dynamic_pointer_cast<CommandBase>(
      std::make_shared<SendSide::CmdResetStream>()));
}

}  // namespace moss
