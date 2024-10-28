#pragma once

#include <cstdint>

struct SuperBlock {
  uint32_t magic;       // Must be FSMAGIC
  uint32_t size;        // Size of file system image (blocks)
  uint32_t nblocks;     // Number of data blocks
  uint32_t ninodes;     // Number of inodes.
  uint32_t nlog;        // Number of log blocks
  uint32_t logstart;    // Block number of first log block
  uint32_t inodestart;  // Block number of first inode block
  uint32_t bmapstart;   // Block number of first free map block
};