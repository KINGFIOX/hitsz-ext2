#pragma once

/// @brief this is the data that should pass to fuse_main
struct XV6FSData {
  int fd;  // file descriptor of the disk image
  char* _mmap_base;
};

void* op_init(struct fuse_conn_info* conn);

void op_destroy(void* userdata);