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
#include "util/util.h"

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

// #include "protoc_test/test.pb.h"

// template <typename T>
// void Ser(T obj, std::stringstream *ss) {
//   obj.SerializeToOstream(ss);
// }

// int protoc_test() {
//   GOOGLE_PROTOBUF_VERIFY_VERSION;

//   tutorial::AddressBook address_book;
//   tutorial::Person *person = address_book.add_people();
//   person->set_id(31);
//   *person->mutable_name() = "Aaron";
//   person->set_email("Robert.Aaron2017@outlook.com");
//   tutorial::Person::PhoneNumber *phone_number = person->add_phones();
//   phone_number->set_number("123");
//   phone_number->set_type(tutorial::Person::WORK);

//   std::cout << address_book.DebugString() << std::endl;

//   std::stringstream ss;
//   // address_book.SerializeToOstream(&ss);
//   Ser(address_book, &ss);
//   std::cout << "LENGTH:" << ss.str().length() << std::endl;
//   tutorial::AddressBook book2;
//   book2.ParseFromIstream(&ss);
//   std::cout << "LENGTH:" << ss.str().length() << std::endl;

//   std::cout << book2.DebugString() << std::endl;

//   google::protobuf::ShutdownProtobufLibrary();
// }

int main(int argc, char **argv) {
  InitLogger(argv);
  // protoc_test();
  UDPClientChannel channel;
  channel.Connect("119.23.51.15", 9877);
  char send_buff[100];
  char recv_buff[100];
  Data send_data(send_buff, 100), recv_data(recv_buff, 100);
  for (int i = 0; i < 100; i++) channel.Send(&send_data, &recv_data);
  return 0;
}
