/**
 * Copyright 2019 Aaron Robert
 * */
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <sstream>
#include <string>

const size_t MAXSIZE = 65535;
char const hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
std::string DataToHex(char *data, size_t len) {
  std::stringstream ss;
  for (int i = 0; i < len; i++) {
    ss << hex_chars[(data[i] & 0xF0) >> 4];
    ss << hex_chars[(data[i] & 0x0F) >> 0];
    ss << ' ';
  }
  return ss.str();
}
int main() {
  int iRet;
  int iSockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (iSockfd == -1) std::cout << "socket error:" << errno << std::endl;
  sockaddr_in *sSvrAddr = new sockaddr_in;
  sockaddr_in *sCliAddr = new sockaddr_in;
  sSvrAddr->sin_family = AF_INET;
  sSvrAddr->sin_addr.s_addr = htonl(INADDR_ANY);
  sSvrAddr->sin_port = htons(9877);
  iRet =
      bind(iSockfd, reinterpret_cast<sockaddr *>(sSvrAddr), sizeof(*sSvrAddr));
  if (iRet == -1) std::cout << "bind error:" << errno << std::endl;
  while (1) {
    int n;
    socklen_t len = sizeof(*sCliAddr);
    char data[MAXSIZE];
    n = recvfrom(iSockfd, data, MAXSIZE, 0,
                 reinterpret_cast<sockaddr *>(sCliAddr), &len);
    if (n == -1) std::cout << "recvfrom error:" << errno << std::endl;
    std::cout << "Recv[" << n << "] " << std::endl;
    // cout << DataToHex(data, n) << endl;
    char ackData[] = {0x01};
    iRet = sendto(iSockfd, ackData, sizeof(ackData), 0,
                  reinterpret_cast<sockaddr *>(sCliAddr), len);
  }
}
