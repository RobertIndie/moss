/**
 * Copyright 2019 Aaron Robert
 * */
#include "common/common.h"

int UDPChannel::Connect(std::string ip, unsigned short port) {
  this->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  this->sa->sin_family = AF_INET;
  // store this IP address in sa:
  inet_pton(AF_INET, ip.c_str(), &(this->sa->sin_addr));
  this->sa->sin_port = htons(port);
}

Data* UDPChannel::Send(Data* data) {}
