#pragma once

#include <mutex>

#include "Block.h"
#include "common.h"

struct Block;

class BlockCache {
 public:
  static std::mutex mtx;
  static Block blocks[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  static Block head;

  static void init(void);

  static Block *block_read(uint32_t dev, uint32_t blockno);
  static void block_release(Block *b);
  static void block_write(Block *b);

  static void block_pin(Block *b);
  static void block_unpin(Block *b);

 private:
  static Block *block_get(uint32_t dev, uint32_t blockno);
};
