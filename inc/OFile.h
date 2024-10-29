#pragma once

#include <sys/types.h>

#include <cstdint>

#include "INode.h"
#include "Pipe.h"
#include "Stat.h"

struct Pipe;

struct OFile {
  enum { FD_NONE, FD_PIPE, FD_INODE, FD_DEVICE } type;
  Pipe *pipe;      // FD_PIPE
  INode *ip;       // FD_INODE and FD_DEVICE
  off_t off;       // FD_INODE
  uint16_t major;  // FD_DEVICE
  bool readable;
  bool writable;

  uint32_t ref;  // reference count

  int stat(Stat *addr);
  int read(char *addr, uint32_t n);
  int write(const char *addr, uint32_t n);
};