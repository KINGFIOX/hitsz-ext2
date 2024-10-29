#pragma once

/// @brief this is the data that should pass to fuse_main
#include "INode.h"

struct XV6FSData {
  int fd;  // file descriptor of the disk image
  char* _mmap_base;
};

INode* inode_name(const char* path);

INode* inode_name_parent(const char* path, char* name);

void* op_init(struct fuse_conn_info* conn);

int op_getattr(const char* path, struct stat* stbuf);

int op_fgetattr(const char* path, struct stat* stbuf, struct fuse_file_info* fi);

int op_open(const char* path, struct fuse_file_info* fi);

void op_destroy(void* userdata);