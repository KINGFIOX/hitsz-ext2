#pragma once

#include <cstdint>
#include <mutex>

#include "DiskINode.h"

struct inode {
  uint32_t dev;    // Device number
  uint32_t inum;   // Inode number
  uint32_t ref;    // Reference count
  std::mutex mtx;  // protects everything below here
  bool valid;      // inode has been read from disk?

  DiskINode dinode;  // copy of disk inode
};