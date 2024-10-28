#include "BlockCache.h"

#include <cassert>
#include <cstdint>
#include <mutex>

#include "Block.h"
#include "common.h"

std::mutex BlockCache::mtx;
Block BlockCache::blocks[NBUF];
Block BlockCache::head;

void BlockCache::init(void) {
  Block *b;

  // Create linked list of buffers
  head.prev = &head;
  head.next = &head;
  for (b = blocks; b < blocks + NBUF; b++) {
    b->next = head.next;
    b->prev = &head;
    head.next->prev = b;
    head.next = b;
  }
}

/// Return a locked buffer with the contents of the indicated block.
Block *BlockCache::block_get(uint32_t dev, uint32_t blockno) {
  Block *b;

  mtx.lock();

  // Is the block already cached?
  for (b = head.next; b != &head; b = b->next) {
    if (b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
      mtx.unlock();
      b->mtx.lock();
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for (b = head.prev; b != &head; b = b->prev) {
    if (b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      mtx.unlock();
      b->mtx.lock();
      return b;
    }
  }

  assert(0 && "no free buffer");
}

Block *BlockCache::block_read(uint32_t dev, uint32_t blockno) {
  Block *b = block_get(dev, blockno);
  if (!b->valid) {
    // TODO virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

void BlockCache::block_write(Block *b) {
  // if (!holdingsleep(&b->lock)) panic("bwrite");
  // TODO virtio_disk_rw(b, 1);
}

void BlockCache::block_release(Block *b) {
  // assert( b.is_locked() );

  b->mtx.unlock();

  mtx.lock();
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = head.next;
    b->prev = &head;
    head.next->prev = b;
    head.next = b;
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
