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

#ifdef CPP11
template <typename TimeType = int64_t>
#else
template <typename TimeType>
#endif
TimeType GetTimestamp() {
  timeval tv;
  gettimeofday(&tv, NULL);
  return static_cast<TimeType>(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
#endif  // SRC_UTIL_UTIL_H_
