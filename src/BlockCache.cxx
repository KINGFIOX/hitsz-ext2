
#include "BlockCache.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <mutex>

#include "Block.h"
#include "PosixEnv.h"
#include "common.h"

void BlockCache::init(void) {
  // do nothing
}

Block *BlockCache::block_get(uint32_t dev, uint32_t blockno) {
  Block *b;

  mtx.lock();

  auto it = blocks.find(BlockKey(dev, blockno));
  if (it != blocks.end()) {
    b = it->second;
    b->refcnt++;
    b->mtx.lock();
    mtx.unlock();
    return b;
  }

  b = new Block();
  b->dev = dev;
  b->blockno = blockno;
  b->valid = false;
  b->refcnt = 1;
  blocks[BlockKey(dev, blockno)] = b;  // insert into map
  b->mtx.lock();
  mtx.unlock();

  return b;
}

Block *BlockCache::block_read(uint32_t dev, uint32_t blockno) {
  Block *b = block_get(dev, blockno);
  if (!b->valid) {
    PosixEnv::read(b);
    b->valid = true;
  }
  assert(b->mtx.try_lock() == false && "return locked block");
  return b;
}

void BlockCache::block_write(Block *b) {
  assert(b->mtx.try_lock() == false && "block is already locked");
  PosixEnv::write(b);
}

void BlockCache::block_release(Block *b) {
  // assert( b.is_locked() );
  assert(b->mtx.try_lock() == false && "block is not locked");

  b->mtx.unlock();

  mtx.lock();
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    blocks.erase(BlockKey(b->dev, b->blockno));
    delete b;
  }

  mtx.unlock();
}

void BlockCache::block_pin(Block *b) {
  mtx.lock();
  b->refcnt++;
  mtx.unlock();
}

void BlockCache::block_unpin(Block *b) {
  mtx.lock();
  b->refcnt--;
  mtx.unlock();
}
