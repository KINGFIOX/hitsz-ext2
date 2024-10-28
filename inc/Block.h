#pragma once

#include <cstdint>
#include <mutex>

#include "common.h"

struct Block {
  bool valid;  // has data been read from disk?
  bool disk;   // does disk "own" buf?
  uint32_t dev;
  uint32_t blockno;
  std::mutex mtx;
  uint32_t refcnt;
  Block *prev;  // LRU cache list
  Block *next;
  uint8_t data[BSIZE];
};
