#pragma once

#include <cstdint>
#include <mutex>

#include "BlockCache.h"
#include "common.h"

class BlockCache;

struct BlockKey {
  uint32_t dev;
  uint32_t blockno;

  BlockKey(uint32_t dev, uint32_t blockno) : dev(dev), blockno(blockno) {}

  bool operator<(const BlockKey &other) const {
    if (dev != other.dev) return dev < other.dev;
    return blockno < other.blockno;
  }
};

struct Block {
  bool valid;  // has data been read from disk?
  // bool disk;   // does disk "own" buf?
  uint32_t dev;
  uint32_t blockno;
  std::mutex mtx;
  uint32_t refcnt;
  uint8_t data[BSIZE];

 private:
  friend class BlockCache;
};
