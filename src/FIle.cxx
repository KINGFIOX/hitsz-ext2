#include <cassert>
#include <cstdint>
#include <cstring>

#include "Log.h"
#include "OFile.h"
#include "common.h"

int OFile::stat(Stat* addr) {
  if (this->type == OFile::FD_INODE || this->type == OFile::FD_DEVICE) {
    INode* ip = this->ip;
    ip->lock();
    Stat st;
    this->ip->stat(&st);
    ip->unlock();
    ::memcpy((void*)addr, &st, sizeof(Stat));
    return 0;
  }
  return -1;
}

int OFile::read(char* addr, uint32_t n) {
  uint32_t r = 0;

  if (this->readable == 0) return -1;

  if (this->type == FD_PIPE) {
    // TODO r = piperead(this->pipe, addr, n);
  } else if (this->type == OFile::FD_DEVICE) {
    // TODO
    // if (this->major < 0 || this->major >= NDEV || !devsw[f->major].read) return -1;
    // r = devsw[f->major].read(1, addr, n);
  } else if (this->type == OFile::FD_INODE) {
    this->ip->lock();
    r = this->ip->read((uint64_t)addr, this->off, n);
    if (r > 0) {
      this->off += r;
    }
    this->ip->unlock();
  } else {
    assert(0 && "ofile read");
  }

  return r;
}

int OFile::write(const char* addr, uint32_t n) {
  int ret = 0;

  if (this->readable == 0) return -1;
  if (this->type == OFile::FD_PIPE) {
    // TODO
  } else if (this->type == OFile::FD_DEVICE) {
    if (this->major < 0 || this->major >= NDEV /* || xxx */) return -1;
    // TODO
  } else if (this->type == OFile::FD_INODE) {
    // write a few blocks at a time to avoid exceeding the maximum log transaction size,
    // including i-node, indirect block, allocation blocks, and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei() might be writing a device like the console.
    uint32_t max = ((MAX_OP_BLOCKS - 1 - 1 - 2) / 2) * BSIZE;

    uint32_t i = 0;
    while (i < n) {
      uint32_t n1 = n - i;
      if (n1 > max) n1 = max;

      Log::begin_op();
      this->ip->lock();
      uint32_t r = this->ip->write((uint64_t)addr + i, this->off, n1);
      if (r > 0) {
        this->off += r;
      }
      this->ip->unlock();
      Log::end_op();

      if (r != n1) {
        break;  // error from ip->write
      }
      i += r;
    }
    ret = (i == n ? n : -1);
  } else {
    assert(0 && "ofile write");
  }
  return ret;
}
