#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>

struct OFile;

#include "OFile.h"
#include "common.h"

struct Pipe {
 private:
  std::mutex mtx;
  std::condition_variable cv_rd;
  std::condition_variable cv_wr;

 public:
  char data[PIPESIZE];
  uint32_t nread;   // number of bytes read
  uint32_t nwrite;  // number of bytes written
  bool readopen;    // read fd is still open
  bool writeopen;   // write fd is still open

  static int alloc(OFile **wr, OFile **rd);
  void close(bool writable);
  int write(const char *addr, uint32_t n);
  int read(char *addr, uint32_t n);
};