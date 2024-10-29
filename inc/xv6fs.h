#pragma once

#include "Logger.h"

/// @brief this is the data that should pass to fuse_main
struct XV6FSData {
  int fd;  // file descriptor of the disk image
  char* _mmap_base;
  Logger* logger;
};

//   debugf("enter %s", e2data->device);
void* xv6fs_init(struct fuse_conn_info* conn);