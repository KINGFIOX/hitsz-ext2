#pragma once

#include <cstdint>

struct Stat {
  uint32_t dev;    // File system's disk device
  uint32_t ino;    // Inode number
  uint16_t type;   // Type of file
  uint16_t nlink;  // Number of links to file
  uint64_t size;   // Size of file in bytes
};
