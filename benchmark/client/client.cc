/**
 * Copyright 2019 Aaron Robert
 * */
#include <stdio.h>
#include <vector>
#include "./login.pb.h"
#include "fstream"
#include "rpc/rpc.h"

void proc(int id, std::fstream &fs_err, std::fstream &fs_result, const char *ip,
          int port) {
  UDPClientChannel channel;
  channel.Connect(ip, port);
  ClientProxy prx(&channel);
  while (1) {
    benchmark_test::LoginMessage req;
    benchmark_test::LoginResult res;
    req.set_id(12138);
    req.set_password("123456");
    prx.Call("Login", &req, &res);
    if (res.code() == 0 && res.error() == "No Error") {
      fs_result << id << ":Sent" << std::endl;
    } else {
      fs_err << id << ":ERROR" << std::endl;
    }
  }
}

int main(int argc, char *argv[]) {
  std::fstream fs_err("./error.txt");
  std::fstream fs_result("./result.txt");
  if (!fs_err.is_open()) {
    printf("output.txt dose not exist!\n");
    return -1;
  }
  if (!fs_result.is_open()) {
    printf("result.txt does not exist!\n");
    return -1;
  }
  const char *ip = argv[1];
  int port = atoi(argv[2]);
  int task = atoi(argv[3]);
  int proccnt = atoi(argv[4]);
  if (argc < 5) {
    printf(
        "Usage:\n"
        "example_echosvr [IP] [PORT] [TASK] [PROCESS_COUNT]\n");
    return -1;
  }

  std::vector<pid_t> pids;
  bool isParent = false;
  for (int i = 0; i < proccnt; i++) {
    pid_t pid = fork();
    if (pid > 0) {
      pids.push_back(pid);
      isParent = true;
    } else if (pid < 0) {
      printf("Create proc error!\n");
    }
    proc(i, fs_err, fs_result, ip, port);
  }
  if (isParent) {
    sleep(10);
    for (std::vector<pid_t>::iterator iter; iter != pids.end(); iter++) {
      kill(*iter, SIGABRT);
    }
  }
  return 0;
}
