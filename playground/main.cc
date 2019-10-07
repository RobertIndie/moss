/*
 * Copyright 2019 AaronRobert Playground
 * */
#include <string.h>
#include <sstream>
#include "iostream"

struct _HEADER {
  unsigned int hs;
};

int main() {
  std::stringstream ss;
  _HEADER hdr;
  hdr.hs = 1;
  char* mem = new char[sizeof(_HEADER)];
  memcpy(mem, &hdr, sizeof(_HEADER));
  // for (int i = 0; i < sizeof(_HEADER); i++) {
  //   ss << mem[i];
  // }
  std::string str(mem, sizeof(_HEADER));
  ss << str;
  ss << "Hello";
  std::cout << ss.str() << std::endl;

  char* mem2 = new char[sizeof(_HEADER)];
  ss.read(mem2, sizeof(_HEADER));
  unsigned int hs2;
  memcpy(&hs2, mem2, sizeof(_HEADER));
  std::cout << hs2 << std::endl;
  // char* mem3 = new char[7];
  // ss.read(mem3, 6);
  // mem3[7] = 0;
  std::string sss;
  ss >> sss;
  std::cout << "PUT" << sss << std::endl;

  return 0;
}
