#pragma once

#include <cstdint>
#include <map>
#include <mutex>

#include "Block.h"
#include "common.h"

struct Block;

struct BlockKey;

class BlockCache {
 public:
  static inline std::mutex mtx;

  static void init(void);

  static Block *block_read(uint32_t dev, uint32_t blockno);
  static void block_release(Block *b);
  static void block_write(Block *b);

  static void block_pin(Block *b);
  static void block_unpin(Block *b);

 private:
  /// @brief Return a locked block with the contents of the indicated block.
  static Block *block_get(uint32_t dev, uint32_t blockno);

 private:
  static inline std::map<BlockKey /*Key*/, Block * /*Value*/> blocks;
};
