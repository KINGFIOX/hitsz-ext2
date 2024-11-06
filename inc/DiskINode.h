#pragma once

#include <cstdint>

#include "common.h"

struct DiskINode {
  enum FileType : uint16_t { Invalid = 0, T_DIR = 1, T_FILE = 2, T_DEVICE = 3 };

  FileType type;                // File type
  uint16_t major;               // Major device number (T_DEVICE only)
  uint16_t minor;               // Minor device number (T_DEVICE only)
  uint16_t nlink;               // Number of links to inode in file system
  uint32_t size;                // Size of file (bytes)
  uint32_t addrs[NDIRECT + 1];  // Data block addresses
};