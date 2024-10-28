#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "Block.h"
#include "BlockCache.h"
#include "Log.h"
#include "SuperBlock.h"
#include "common.h"

/// @brief clear the block(disk)'s data
static void block_zeroed(uint32_t dev, uint32_t bno) {
  Block *bp = BlockCache::block_read(dev, bno);
  ::memset(bp->data, 0, BSIZE);
  Log::log_write(bp);
  BlockCache::block_release(bp);
}

/// @brief Allocate a zeroed disk block.
/// @return bno; 0 if out of disk space.
static uint32_t block_alloc(uint32_t dev) {
  for (size_t b = 0; b < SuperBlock::size; b += BIT_PER_BLOCK) {
    Block *bp = BlockCache::block_read(dev, BIT_BLOCK(b));
    for (size_t bi = 0; bi < BIT_PER_BLOCK && b + bi < SuperBlock::size; bi++) {
      size_t m = 1 << (bi % 8);           // mask
      if ((bp->data[bi / 8] & m) == 0) {  // Is block free?
        bp->data[bi / 8] |= m;            // Mark block in use.
        Log::log_write(bp);
        BlockCache::block_release(bp);
        block_zeroed(dev, b + bi);
        return b + bi;
      }
    }
    BlockCache::block_release(bp);
  }
  ::printf("balloc: out of blocks\n");
  return 0;
}

/// @brief Free a disk block. only set the bit to 0 in bitmap.
static void block_free(uint32_t dev, uint32_t b) {
  Block *bp = BlockCache::block_read(dev, BIT_BLOCK(b));
  uint32_t bi = b % BIT_PER_BLOCK;
  uint32_t m = 1 << (bi % 8);
  assert((bp->data[bi / 8] & m) && "freeing free block");
  bp->data[bi / 8] &= ~m;
  Log::log_write(bp);
  BlockCache::block_release(bp);
}