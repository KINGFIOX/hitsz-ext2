#pragma once

#include <cstdint>

/// @brief super block(disk && mem)
class SuperBlock {
 public:
  static uint32_t magic;       // Must be FSMAGIC
  static uint32_t size;        // Size of file system image (blocks)
  static uint32_t nblocks;     // Number of data blocks
  static uint32_t ninodes;     // Number of inodes.
  static uint32_t nlog;        // Number of log blocks
  static uint32_t logstart;    // Block number of first log block
  static uint32_t inodestart;  // Block number of first inode block
  static uint32_t bmapstart;   // Block number of first free map block

  static void init(uint32_t dev);
};