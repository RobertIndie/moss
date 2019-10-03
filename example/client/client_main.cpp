/**
 * Copyright 2019 Aaron Robert
 * */
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "common/common.h"
const size_t MAXSIZE = 4096;
int readable_timeo(int fd, int sec) {
  fd_set rset;
  timeval tv;
  FD_ZERO(&rset);
  FD_SET(fd, &rset);
  tv.tv_sec = sec;
  tv.tv_usec = 0;
  return select(fd + 1, &rset, NULL, NULL, &tv);
}

int old_main() {
  int totalPacket = 0, ackPacket = 0;
  int iSockfd = socket(AF_INET, SOCK_DGRAM, 0);
  sockaddr_in *sSvrAddr = new sockaddr_in;
  sSvrAddr->sin_family = AF_INET;
  sSvrAddr->sin_addr.s_addr = htonl(1998009103);
  sSvrAddr->sin_port = htons(9877);
  int n;
  char data[] = {0x01, 0x02};
  char data2[50000];
  for (int i = 0; i < 2; i++) {
    std::cout << "ACK/TOTAL: " << ackPacket << "/" << totalPacket << " "
              << ackPacket * 1.0 / totalPacket << std::endl;
    totalPacket++;
    n = sendto(iSockfd, data2, sizeof(data2), 0,
               reinterpret_cast<sockaddr *>(sSvrAddr), sizeof(*sSvrAddr));
    if (n == -1) continue;
    char ackData[1];
    if (readable_timeo(iSockfd, 1) == 0) {
      std::cout << "Timeout" << std::endl;
      continue;
    }
    n = recvfrom(iSockfd, ackData, sizeof(ackData), 0, NULL, NULL);
    if (n == -1) continue;
    if (ackData[0] == 0x01) ackPacket++;
  }
}

int main() {
  UDPClientChannel channel;
  channel.Connect("0.0.0.0", 9877);
  char send_buff[100];
  char recv_buff[100];
  Data send_data, recv_data;
  send_data.buff = send_buff;
  send_data.len = 100;
  recv_data.buff = recv_buff;
  recv_data.len = 100;
  channel.Send(&send_data, &recv_data);
  return 0;
}
