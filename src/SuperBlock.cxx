#include "SuperBlock.h"

#include <cassert>
#include <cstdint>
#include <cstring>

#include "Block.h"
#include "BlockCache.h"
#include "Log.h"

uint32_t SuperBlock::magic;       // Must be FSMAGIC
uint32_t SuperBlock::size;        // Size of file system image (blocks)
uint32_t SuperBlock::nblocks;     // Number of data blocks
uint32_t SuperBlock::ninodes;     // Number of inodes.
uint32_t SuperBlock::nlog;        // Number of log blocks
uint32_t SuperBlock::logstart;    // Block number of first log block
uint32_t SuperBlock::inodestart;  // Block number of first inode block
uint32_t SuperBlock::bmapstart;   // Block number of first free map block

void SuperBlock::init(uint32_t dev) {
  Block *bp = BlockCache::block_read(dev, 1);  // bno of super block is 1
  uint32_t *sb = (uint32_t *)bp->data;

  SuperBlock::magic = sb[0];
  SuperBlock::size = sb[1];
  SuperBlock::nblocks = sb[2];
  SuperBlock::ninodes = sb[3];
  SuperBlock::nlog = sb[4];
  SuperBlock::logstart = sb[5];
  SuperBlock::inodestart = sb[6];
  SuperBlock::bmapstart = sb[7];

  BlockCache::block_release(bp);

  assert(SuperBlock::magic == FS_MAGIC && "invalid file system");

  Log::init(dev);
}