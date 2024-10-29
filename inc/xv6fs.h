#pragma once

#include "Logger.h"

/// @brief this is the data that should pass to fuse_main
struct xv6fs_data {
  int fd;  // file descriptor of the disk image
  char* _mmap_base;
  Logger* logger;
};