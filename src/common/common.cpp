/**
 * Copyright 2019 Aaron Robert
 * */
#include "common/common.h"

#pragma region RTTInfo
void RTTInfo::Init() {
  this->time_base = GetTimestamp();
  this->rtt = 0;
  this->srtt = 0;
  this->rttvar = 50;
  this->rto = this->GetRTO();
}

void RTTInfo::NewPack() { this->retransmitted_count = 0; }

float RTTInfo::GetRTO() {
  float rto = this->srtt + (4.0 * this->rttvar);
  if (rto < kRXTMin)
    rto = kRXTMin;
  else if (rto > kRXTMax)
    rto = kRXTMax;
  return rto;
}

template <typename TimeType>
TimeType RTTInfo::GetRelativeTs() {
  int64_t ts = GetTimestamp();
  return static_cast<TimeType>(ts - this->time_base);
}

template <typename TimeType>
TimeType RTTInfo::Start() {
  return static_cast<TimeType>(
      this->rto + 0.5);  // if TimeType is integer, round float to integer
}

int RTTInfo::Timeout() {
  this->rto *= 2;
  if (++this->retransmitted_count > kRXTMaxTimes) {
    return -1;  // give up sending this packet
  }
  return 0;
}

template <typename TimeType>
void RTTInfo::Stop(TimeType rtt) {
  this->rtt = rtt;
  double delta = this->rtt - this->srtt;
  this->srtt += delta / 8;
  if (delta < 0.0) delta = -delta;
  this->rttvar += (delta - this->rttvar) / 4;
  this->rto = this->GetRTO();
}
#pragma endregion

#pragma region PacketBuilder
PacketBuilder::PacketBuilder(sockaddr_in *sa) {
  this->result = new msghdr;
  this->result->msg_control = NULL;
  this->result->msg_controllen = 0;
  this->result->msg_flags = 0;
  if (sa != nullptr) {
    this->result->msg_name = sa;
    this->result->msg_namelen = sizeof(*sa);
  } else {
    sockaddr_in *new_sa = new sockaddr_in;
    this->result->msg_name = new_sa;
    this->result->msg_namelen = sizeof(*new_sa);
  }
#define IOV_LEN 2
  iovec *iov = new iovec[IOV_LEN];
  _iov = iov;  // for memory release
  this->result->msg_iov = iov;
  this->result->msg_iovlen = IOV_LEN;
}

Header *PacketBuilder::MakeHeader(uint32_t seq, uint32_t ts) {
  Header *hdr = new Header;
  _header = hdr;
  hdr->seq = seq;
  hdr->ts = ts;
  result->msg_iov[0].iov_base = hdr;
  result->msg_iov[0].iov_len = sizeof(*hdr);
  return hdr;
}

Data *PacketBuilder::MakeData(char *buff, size_t buff_size) {
  Data *data;
  if (buff != nullptr)
    data = new Data(buff, buff_size);
  else
    data = new Data(buff_size);
  _data = data;
  result->msg_iov[1].iov_base = data->buff;
  result->msg_iov[1].iov_len = data->len;
  return data;
}

Data *PacketBuilder::MakeData(Data *data) {
  result->msg_iov[1].iov_base = data->buff;
  result->msg_iov[1].iov_len = data->len;
  return data;
}

msghdr *PacketBuilder::GetResult() const { return result; }

PacketBuilder::~PacketBuilder() {
  DELETE_PTR(_iov);
  DELETE_PTR(_header);
  DELETE_PTR(_data);
  DELETE_PTR(result);
}
#pragma endregion

#pragma region UDPChannel
void UDPChannel::SocketConnect(std::string ip, unsigned short port) {
  this->socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  this->sa_->sin_family = AF_INET;
  // store this IP address in sa:
  inet_pton(AF_INET, ip.c_str(), &(this->sa_->sin_addr));
  this->sa_->sin_port = htons(port);
}

void UDPChannel::SocketBind() {
  bind(this->socket_fd_, reinterpret_cast<sockaddr *>(this->sa_),
       sizeof(*this->sa_));
}
#pragma endregion

#pragma region UDPClientChannel
int UDPClientChannel::Connect(std::string ip, unsigned short port) {
  this->SocketConnect(ip, port);
}

int UDPClientChannel::Send(Data *in_data, Data *out_data) {
  if (this->reinit_rtt) {
    this->rtt_info_.Init();
  }

  msghdr *msgsend, *msgrecv;
  // msgsend.msg_control = NULL;
  // msgsend.msg_controllen = 0;
  // msgsend.msg_flags = 0;
  // iovec iovsend[2], iovrecv[2];
  Header *sendhdr, *recvhdr;
  // msgsend.msg_name = this->sa_;
  // msgsend.msg_namelen = sizeof(*this->sa_);

  // // set iovec for msgsend
  // iovsend[0].iov_base = &sendhdr;
  // iovsend[0].iov_len = sizeof(Header);
  // iovsend[1].iov_base = in_data->buff;
  // iovsend[1].iov_len = in_data->len;
  // msgsend.msg_iov = iovsend;
  // msgsend.msg_iovlen = 2;

  PacketBuilder pbsend(this->sa_);
  sendhdr = pbsend.MakeHeader();
  pbsend.MakeData(in_data);
  msgsend = pbsend.GetResult();

  // set iovec for msgrecv
  // iovrecv[0].iov_base = &recvhdr;
  // iovrecv[0].iov_len = sizeof(Header);
  // iovrecv[1].iov_base = out_data->buff;
  // iovrecv[1].iov_len = out_data->len;
  // msgrecv.msg_iov = iovrecv;
  // msgrecv.msg_iovlen = 2;

  PacketBuilder pbrecv(nullptr);
  recvhdr = pbrecv.MakeHeader();
  pbrecv.MakeData(out_data);
  msgrecv = pbrecv.GetResult();

  this->rtt_info_.NewPack();

  ssize_t recvSize = 0;
  bool isSendAgain = true;
  do {
    isSendAgain = false;
    sendhdr->seq = 1;
    sendhdr->ts = this->rtt_info_.GetRelativeTs();
    std::cout << "Send:" << sendhdr->ts << std::endl;
    sendmsg(this->socket_fd_, msgsend, 0);
    int waitTime = this->rtt_info_.Start();
    bool isContinueWait = false;
    do {
      isContinueWait = false;
      if (ReadableTimeout(this->socket_fd_, waitTime) == 0) {
        // timeout
        if (this->rtt_info_.Timeout() < 0) {
          this->reinit_rtt =
              true;   // reinit rtt_info in case we're called again
          return -1;  // send error
        }
        isSendAgain = true;
      } else {
        ssize_t recvSize = recvmsg(this->socket_fd_, msgrecv, 0);
        if (recvSize < sizeof(Header) || recvhdr->seq != sendhdr->seq) {
          waitTime -= this->rtt_info_.GetRelativeTs();
          if (waitTime < 0) break;
          isContinueWait = true;
        }
      }
    } while (isContinueWait);
  } while (isSendAgain);
  // Send and recv packet success
  this->rtt_info_.Stop(this->rtt_info_.GetRelativeTs() - recvhdr->ts);
  return (recvSize - sizeof(Header));
}
#pragma endregion

int UDPServerChannel::Bind(std::string ip, unsigned short port) {
  this->SocketConnect(ip, port);
  this->SocketBind();
}

int UDPServerChannel::Serve(ServeFunc serve_func) {
  sockaddr_in *cliAddr = new sockaddr_in;
  while (1) {
    socklen_t len = sizeof(*cliAddr);
    PacketBuilder pbrecv(nullptr);
    Header *recvhdr = pbrecv.MakeHeader();
    pbrecv.MakeData(nullptr, 1452);
    msghdr *msgrecv = pbrecv.GetResult();
    ssize_t recvSize = recvmsg(this->socket_fd_, msgrecv, 0);
    PacketBuilder pbsend(reinterpret_cast<sockaddr_in *>(msgrecv->msg_name));
    pbsend.MakeHeader(recvhdr->seq, recvhdr->ts);  // todo
    pbsend.MakeData(nullptr, 1);
    msghdr *msgsend = pbsend.GetResult();
    sendmsg(this->socket_fd_, msgsend, 0);
  }
  delete cliAddr;
}
