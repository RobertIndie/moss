/**
 * Copyright 2019 Aaron Robert
 * */
#include "common/common.h"

template <typename TimeType>
RTTInfo<TimeType>::RTTInfo() {
  this->Init();
}

template <typename TimeType>
void RTTInfo<TimeType>::Init() {
  this->time_base = GetTimestamp();
  this->rtt = 0;
  this->srtt = 0;
  this->rttvar = 0.75;
  this->rto = this->GetRTO();
}

template <typename TimeType>
void RTTInfo<TimeType>::NewPack() {
  this->retransmitted_count = 0;
}

template <typename TimeType>
float RTTInfo<TimeType>::GetRTO() {
  float rto = this->srtt + (4.0 * this->rttvar);
  if (rto < kRXTMin)
    rto = kRXTMin;
  else if (rto > kRXTMax)
    rto = kRXTMax;
  return rto;
}

template <typename TimeType>
TimeType RTTInfo<TimeType>::GetRelativeTs() {
  int64_t ts = GetTimestamp();
  return static_cast<TimeType>(ts - this->time_base);
}

template <typename TimeType>
TimeType RTTInfo<TimeType>::Start() {
  return static_cast<TimeType>(
      this->GetRTO() + 0.5);  // if TimeType is integer, round float to integer
}

template <typename TimeType>
int RTTInfo<TimeType>::Timeout() {
  this->rto *= 2;
  if (++this->retransmitted_count > kRXTMaxTimes) {
    return -1;  // give up sending this packet
  }
  return 0;
}

template <typename TimeType>
void RTTInfo<TimeType>::Stop(TimeType rtt) {
  this->rtt = rtt;
  double delta = this->rtt - this->srtt;
  this->srtt += delta / 8;
  if (delta < 0.0) delta = -delta;
  this->rttvar += (delta - this->rttvar) / 4;
  this->rto = this->GetRTO();
}

int UDPChannel::Connect(std::string ip, unsigned short port) {
  this->socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  this->sa_->sin_family = AF_INET;
  // store this IP address in sa:
  inet_pton(AF_INET, ip.c_str(), &(this->sa_->sin_addr));
  this->sa_->sin_port = htons(port);
}

int UDPChannel::Send(Data* in_data, Data* out_data) {
  if (this->reinit_rtt) {
    this->rtt_info_.Init();
  }

  msghdr msgsend, msgrecv;
  iovec iovsend[2], iovrecv[2];
  hdr sendhdr, recvhdr;
  msgsend.msg_name = this->sa_;
  msgsend.msg_namelen = sizeof(this->sa_);

  // set iovec for msgsend
  iovsend[0].iov_base = &sendhdr;
  iovsend[0].iov_len = sizeof(hdr);
  iovsend[1].iov_base = in_data->buff;
  iovsend[1].iov_len = in_data->len;
  msgsend.msg_iov = iovsend;
  msgsend.msg_iovlen = 2;

  // set iovec for msgrecv
  iovrecv[0].iov_base = &recvhdr;
  iovrecv[0].iov_len = sizeof(hdr);
  iovrecv[1].iov_base = out_data->buff;
  iovrecv[1].iov_len = out_data->len;
  msgrecv.msg_iov = iovrecv;
  msgrecv.msg_iovlen = 2;

  this->rtt_info_.NewPack();
  sendhdr.ts = this->rtt_info_.GetRelativeTs();
  sendmsg(this->socket_fd_, &msgsend, 0);
  ssize_t recvSize = 0;
  bool isSendAgain = true;
  while (isSendAgain) {
    isSendAgain = false;
    if (ReadableTimeout(this->socket_fd_, this->rtt_info_.Start())) {
      // timeout
      if (this->rtt_info_.Timeout() < 0) {
        this->reinit_rtt = true;  // reinit rtt_info in case we're called again
        return -1;                // send error
      }
      isSendAgain = true;
    } else {
      ssize_t recvSize = recvmsg(this->socket_fd_, &msgrecv, 0);
      if (recvSize < sizeof(hdr) || recvhdr.seq != sendhdr.seq) {
        isSendAgain = true;
      }
    }
  }
  // Send and recv packet success
  this->rtt_info_.Stop(this->rtt_info_.GetRelativeTs() - recvhdr.ts);
  return (recvSize - sizeof(hdr));
}
