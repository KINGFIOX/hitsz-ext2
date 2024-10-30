#include <cerrno>

#include "Logger.h"
#include "OFile.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  OFile *fp = (OFile *)fi->fh;  // 有个问题, 这里的 path 没有用到
  if (fp == nullptr) {
    return -ENOENT;
  }
  if (offset > fp->ip->dinode.size) {
    return 0;
  }
  fp->off = offset;  // FIXME
  fp->read(buf, size);
  return size;
}