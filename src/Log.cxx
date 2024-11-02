#include "Log.h"

#include <complex.h>

#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>

#include "Block.h"
#include "BlockCache.h"
#include "SuperBlock.h"
#include "common.h"

std::mutex Log::mtx;
std::condition_variable Log::cv;

void Log::read_head(void) {
  Block *blk = BlockCache::block_read(dev, start);
  LogHeader *hdr = (LogHeader *)(blk->data);
  log_hdr.n = hdr->n;
  for (size_t i = 0; i < log_hdr.n; i++) {
    log_hdr.block[i] = hdr->block[i];
  }
  BlockCache::block_release(blk);
}

void Log::write_head(void) {
  Block *blk = BlockCache::block_read(dev, start);
  LogHeader *hb = (LogHeader *)(blk->data);
  hb->n = log_hdr.n;
  for (size_t i = 0; i < log_hdr.n; i++) {
    hb->block[i] = log_hdr.block[i];
  }
  BlockCache::block_write(blk);
  BlockCache::block_release(blk);
}

void Log::recover_from_log(void) {
  read_head();
  for (size_t tail = 0; tail < log_hdr.n; tail++) {
    Block *log_blk = BlockCache::block_read(dev, start + tail + 1);     // read log block
    Block *dst_blk = BlockCache::block_read(dev, log_hdr.block[tail]);  // read dst
    ::memmove(dst_blk->data, log_blk->data, BSIZE);                     // copy block to dst
    BlockCache::block_write(dst_blk);                                   // write dst to disk
    // 这里与 commit 相比, 少了 block_unpin, 因为这里 log_hdr(mem) 中没有 ref
    BlockCache::block_release(log_blk);
    BlockCache::block_release(dst_blk);
  }
  log_hdr.n = 0;
  write_head();  // clear the log
}

void Log::commit() {
  if (log_hdr.n > 0) {
    write_log();   // Write modified blocks from cache to log
    write_head();  // Write header to disk -- the real commit
    for (size_t tail = 0; tail < log_hdr.n; tail++) {
      Block *log_blk = BlockCache::block_read(dev, start + tail + 1);     // read log block
      Block *dst_blk = BlockCache::block_read(dev, log_hdr.block[tail]);  // read dst
      ::memmove(dst_blk->data, log_blk->data, BSIZE);                     // copy block to dst
      BlockCache::block_write(dst_blk);                                   // write dst to disk
      BlockCache::block_unpin(dst_blk);                                   // because of clearing the log
      BlockCache::block_release(log_blk);
      BlockCache::block_release(dst_blk);
    }
    log_hdr.n = 0;
    write_head();  // Erase the transaction from the log
  }
}

void Log::write_log(void) {
  for (size_t tail = 0; tail < log_hdr.n; tail++) {
    Block *dst_blk = BlockCache::block_read(dev, start + tail + 1);     // log block
    Block *src_blk = BlockCache::block_read(dev, log_hdr.block[tail]);  // cache block
    ::memmove(dst_blk->data, src_blk->data, BSIZE);
    BlockCache::block_write(dst_blk);  // write the log
    BlockCache::block_release(src_blk);
    BlockCache::block_release(dst_blk);
  }
}

void Log::init(uint32_t _dev) {
  assert(sizeof(LogHeader) < BSIZE && "LogHeader too big");

  start = SuperBlock::logstart;
  size = SuperBlock::nlog;
  dev = _dev;

  recover_from_log();
}

void Log::begin_op(void) {
  std::unique_lock<std::mutex> lk_grd(mtx);
  while (1) {
    if (committing) {
      cv.wait(lk_grd);
    } else if (log_hdr.n + (outstanding + 1) * MAX_OP_BLOCKS > LOG_SIZE) {
      // this op might exhaust log space; wait for commit.
      cv.wait(lk_grd);
    } else {
      outstanding += 1;
      // drop lk_grd
      return;
    }
  }
}

void Log::end_op(void) {
  bool do_commit = false;

  mtx.lock();
  outstanding -= 1;
  assert(!committing && "committing");
  if (outstanding == 0) {
    do_commit = 1;
    committing = true;
  } else {
    // begin_op() may be waiting for log space,
    // and decrementing log.outstanding has decreased
    // the amount of reserved space.
    // wakeup(&log);
    cv.notify_all();
  }
  mtx.unlock();
  // release(&log.lock);

  if (do_commit) {
    // call commit w/o holding locks, since not allowed
    // to sleep with locks.
    commit();
    mtx.lock();
    committing = false;
    cv.notify_all();
    mtx.unlock();
  }
}

void Log::log_write(Block *b) {
  mtx.lock();
  assert(log_hdr.n < LOG_SIZE && log_hdr.n < size - 1 && "too big a transaction");
  assert(outstanding >= 1 && "log_write outside of trans");

  size_t i = 0;
  for (i = 0; i < log_hdr.n; i++) {
    if (log_hdr.block[i] == b->blockno) break;  // log absorption
  }
  log_hdr.block[i] = b->blockno;
  if (i == log_hdr.n) {  // Add new block to log?
    BlockCache::block_pin(b);
    log_hdr.n++;
  }
  mtx.unlock();
}
