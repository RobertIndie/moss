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
    _new_sa = new_sa;
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

Data *const PacketBuilder::MakeData(Data *const data) {
  result->msg_iov[1].iov_base = data->buff;
  result->msg_iov[1].iov_len = data->len;
  return data;
}

msghdr *const PacketBuilder::GetResult() const { return result; }

Data *const PacketBuilder::GetData() const { return _data; }

PacketBuilder::~PacketBuilder() {
  DELETE_PTR(_new_sa);
  DELETE_PTR(_iov);
  DELETE_PTR(_header);
  DELETE_PTR(_data);
  DELETE_PTR(result);
}
#pragma endregion

#pragma region UDPChannel
void UDPChannel::SocketConnect(std::string ip, unsigned short port) {
  int ret = 0;
  LOG(INFO) << "Socket Connect: IP:" << ip << "\tPort:" << port;
  ret = socket(AF_INET, SOCK_DGRAM, 0);
  if (ret == -1) PLOG(ERROR);
  this->socket_fd_ = ret;
  this->sa_->sin_family = AF_INET;
  // store this IP address in sa:
  inet_pton(AF_INET, ip.c_str(), &(this->sa_->sin_addr));
  this->sa_->sin_port = htons(port);
}

void UDPChannel::SocketBind() {
  int ret = 0;
  ret = bind(this->socket_fd_, reinterpret_cast<sockaddr *>(this->sa_),
             sizeof(*this->sa_));
  if (ret < 0) PLOG(FATAL);
}
#pragma endregion

#pragma region UDPClientChannel
int UDPClientChannel::Connect(std::string ip, unsigned short port) {
  this->SocketConnect(ip, port);
}

int UDPClientChannel::Send(Data *in_data, Data *out_data) {
  timespec ts_start, ts_end;
  clock_gettime(CLOCK_MONOTONIC, &ts_start);
  int ret = 0;
  if (this->reinit_rtt) {
    this->rtt_info_.Init();
  }

  msghdr *msgsend, *msgrecv;
  Header *sendhdr, *recvhdr;

  PacketBuilder pbsend(this->sa_);
  sendhdr = pbsend.MakeHeader();
  pbsend.MakeData(in_data);
  msgsend = pbsend.GetResult();

  PacketBuilder pbrecv(nullptr);
  recvhdr = pbrecv.MakeHeader();
  pbrecv.MakeData(out_data);
  msgrecv = pbrecv.GetResult();

  this->rtt_info_.NewPack();

  ssize_t recvSize = 0;
  bool isSendAgain = true;
  sendhdr->seq = this->seq_++;
  do {
    isSendAgain = false;
    sendhdr->ts = this->rtt_info_.GetRelativeTs();
    ret = sendmsg(this->socket_fd_, msgsend, 0);
    if (ret == -1) PLOG(ERROR);
    int waitTime = this->rtt_info_.Start();
    DLOG(INFO) << "Client send:" << LOG_VALUE(sendhdr->seq)
               << LOG_VALUE(waitTime);
    bool isContinueWait = false;
    do {
      isContinueWait = false;
      if (ReadableTimeout(this->socket_fd_, waitTime) == 0) {
        DLOG(INFO) << "Timeout:" << LOG_VALUE(sendhdr->seq)
                   << LOG_VALUE(this->rtt_info_.retransmitted_count);
        // timeout
        if (this->rtt_info_.Timeout() < 0) {
          DLOG(ERROR) << "Send error.";
          this->reinit_rtt =
              true;   // reinit rtt_info in case we're called again
          return -1;  // send error
        }
        isSendAgain = true;
      } else {
        recvSize = recvmsg(this->socket_fd_, msgrecv, 0);
        if (recvSize == -1) PLOG(ERROR);
        if (recvSize < sizeof(Header) || recvhdr->seq != sendhdr->seq ||
            memcmp(reinterpret_cast<sockaddr_in *>(msgrecv->msg_name),
                   reinterpret_cast<sockaddr_in *>(msgsend->msg_name),
                   sizeof(sockaddr_in)) != 0) {
          DLOG(ERROR) << "Receive packet error:" << LOG_VALUE(recvSize)
                      << LOG_VALUE(recvhdr->seq) << LOG_VALUE(sendhdr->seq);
          waitTime -= this->rtt_info_.GetRelativeTs();
          if (waitTime < 0) break;
          isContinueWait = true;
        }
      }
    } while (isContinueWait);
  } while (isSendAgain);
  clock_gettime(CLOCK_MONOTONIC, &ts_end);
  // Send and recv packet success
  DLOG(INFO) << "Send success:" << LOG_VALUE(sendhdr->seq)
             << LOG_NV("delta_tv_msec",
                       (ts_end.tv_sec - ts_start.tv_sec) * 1000 +
                           (ts_end.tv_nsec - ts_start.tv_nsec) / 1000000);
  this->rtt_info_.Stop(this->rtt_info_.GetRelativeTs() - recvhdr->ts);
  return (recvSize - sizeof(Header));
}
#pragma endregion

int UDPServerChannel::Bind(std::string ip, unsigned short port) {
  this->SocketConnect(ip, port);
  this->SocketBind();
}

int UDPServerChannel::Serve(ServeFunc serve_func) {
  if (serve_func == nullptr) LOG(FATAL) << "serve_func is null";
  int ret = 0;
  while (1) {
    PacketBuilder pbrecv(nullptr);
    Header *recvhdr = pbrecv.MakeHeader();
    pbrecv.MakeData(nullptr, 1452);
    msghdr *msgrecv = pbrecv.GetResult();
    ssize_t recvSize = recvmsg(this->socket_fd_, msgrecv, 0);
    if (recvSize == -1) PLOG(ERROR);
    PacketBuilder pbsend(reinterpret_cast<sockaddr_in *>(msgrecv->msg_name));
    pbsend.MakeHeader(recvhdr->seq, recvhdr->ts);
    Data *response = serve_func(pbrecv.GetData());
    pbsend.MakeData(response);
    msghdr *msgsend = pbsend.GetResult();
    DLOG(INFO)
        << "Server recv and send:"
        << LOG_NV("origin_ip:",
                  inet_ntoa(reinterpret_cast<sockaddr_in *>(msgrecv->msg_name)
                                ->sin_addr))
        << LOG_NV("origin_port",
                  ntohs(reinterpret_cast<sockaddr_in *>(msgrecv->msg_name)
                            ->sin_port))
        << LOG_VALUE(recvSize) << LOG_NV("seq", recvhdr->seq);
    ret = sendmsg(this->socket_fd_, msgsend, 0);
    if (ret == -1) PLOG(ERROR);
    DELETE_PTR(response);
  }
}
