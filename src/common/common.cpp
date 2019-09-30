/**
 * Copyright 2019 Aaron Robert
 * */
#include "common/common.h"

template <typename TimeType, typename CountType>
RTTInfo<TimeType, CountType>::RTTInfo() {
  this->time_base = GetTimestamp();
}

template <typename TimeType, typename CountType>
TimeType RTTInfo<TimeType, CountType>::MinMax() {}

template <typename TimeType, typename CountType>
void RTTInfo<TimeType, CountType>::NewPack() {
  this->retransmitted_count = 0;
}

int UDPChannel::Connect(std::string ip, unsigned short port) {
  this->socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  this->sa_->sin_family = AF_INET;
  // store this IP address in sa:
  inet_pton(AF_INET, ip.c_str(), &(this->sa_->sin_addr));
  this->sa_->sin_port = htons(port);
}

Data* UDPChannel::Send(Data* in_data, Data* out_data) {
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
}
