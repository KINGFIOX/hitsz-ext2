#pragma once

#include <cstring>
#include <mutex>

#include "Block.h"
class PosixEnv {
 public:
  static void read(Block *blk) {
    size_t offset = blk->blockno * BSIZE;
    std::lock_guard<std::mutex> guard(mtx);
    ::memmove(blk->data, _mmap_base + offset, BSIZE);
  }

  static void write(Block *blk) {
    size_t offset = blk->blockno * BSIZE;
    std::lock_guard<std::mutex> guard(mtx);
    ::memmove(_mmap_base + offset, blk->data, BSIZE);
  }

 private:
  friend void *xv6fs_init(struct fuse_conn_info *conn);
  static inline char *_mmap_base;
  static std::mutex mtx;
};