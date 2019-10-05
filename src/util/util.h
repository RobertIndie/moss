/**
 * Copyright 2019 Aaron Robert
 * */
#ifndef SRC_UTIL_UTIL_H_
#define SRC_UTIL_UTIL_H_
#if __cplusplus >= 201103L
#define CPP11
#endif
#include <stdint.h>
#include <sys/time.h>
#include "glog/logging.h"

inline void InitLogger(char *argv[]) {
  FLAGS_logtostderr = 1;  // log to console
  google::InitGoogleLogging(argv[0]);
}

#define LOG_VALUE(value) #value << "=" << value << " "
#define LOG_NV(name, value) name << "=" << value << " "

#define DELETE_PTR(ptr) \
  if (ptr != nullptr) delete ptr, ptr = nullptr

#define DELETE_ARRAY_PTR(ptr) \
  if (ptr != nullptr) delete[] ptr, ptr = nullptr

inline int64_t GetTimestamp() {
  timeval tv;
  gettimeofday(&tv, NULL);
  return static_cast<int64_t>(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

inline int ReadableTimeout(int fd, timeval *tv) {
  fd_set rset;
  FD_ZERO(&rset);
  FD_SET(fd, &rset);
  return select(fd + 1, &rset, NULL, NULL, tv);
}

inline int ReadableTimeout(int fd, int sec, int usec) {
  timeval tv;
  tv.tv_sec = sec;
  tv.tv_usec = usec;
  return ReadableTimeout(fd, &tv);
}

inline int ReadableTimeout(int fd, int msec) {
  return ReadableTimeout(fd, msec / 1000, msec % 1000 * 1000);
}

typedef unsigned int HashName;

#endif  // SRC_UTIL_UTIL_H_
