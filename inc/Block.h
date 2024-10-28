#pragma once

#include <cstdint>
#include <mutex>

#include "BlockCache.h"
#include "common.h"

struct Block {
  bool valid;  // has data been read from disk?
  bool disk;   // does disk "own" buf?
  uint32_t dev;
  uint32_t blockno;
  std::mutex mtx;
  uint32_t refcnt;
  uint8_t data[BSIZE];

 private:
  friend class BlockCache;
  Block *prev;  // LRU cache list
  Block *next;
};
