#include <cassert>
#include <cstdint>
#include <mutex>

#include "Block.h"
#include "common.h"

namespace BlockCache {

static struct {
  std::mutex mtx;
  Block buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  Block head;
} block_cache;

void blockcache_init(void) {
  Block *b;

  // Create linked list of buffers
  block_cache.head.prev = &block_cache.head;
  block_cache.head.next = &block_cache.head;
  for (b = block_cache.buf; b < block_cache.buf + NBUF; b++) {
    b->next = block_cache.head.next;
    b->prev = &block_cache.head;
    block_cache.head.next->prev = b;
    block_cache.head.next = b;
  }
}

/// Return a locked buffer with the contents of the indicated block.
static Block *block_get(uint32_t dev, uint32_t blockno) {
  Block *b;

  block_cache.mtx.lock();

  // Is the block already cached?
  for (b = block_cache.head.next; b != &block_cache.head; b = b->next) {
    if (b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
      block_cache.mtx.unlock();
      b->mtx.lock();
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for (b = block_cache.head.prev; b != &block_cache.head; b = b->prev) {
    if (b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      block_cache.mtx.unlock();
      b->mtx.lock();
      return b;
    }
  }

  assert(0 && "no free buffer");
}

Block *block_read(uint32_t dev, uint32_t blockno) {
  Block *b = block_get(dev, blockno);
  if (!b->valid) {
    // TODO virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

void block_release(Block *b) {
  // assert( b.is_locked() );

  b->mtx.unlock();

  block_cache.mtx.lock();
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = block_cache.head.next;
    b->prev = &block_cache.head;
    block_cache.head.next->prev = b;
    block_cache.head.next = b;
  }

  block_cache.mtx.unlock();
}

void block_pin(Block *b) {
  block_cache.mtx.lock();
  b->refcnt++;
  block_cache.mtx.unlock();
}

void block_unpin(Block *b) {
  block_cache.mtx.lock();
  b->refcnt--;
  block_cache.mtx.unlock();
}

}  // namespace BlockCache