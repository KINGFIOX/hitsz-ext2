#pragma once

#include <cstdint>
#include <mutex>

#include "common.h"

struct Pipe {
  char data[PIPESIZE];
  uint32_t nread;   // number of bytes read
  uint32_t nwrite;  // number of bytes written
  bool readopen;    // read fd is still open
  bool writeopen;   // write fd is still open

  std::mutex mtx;
};