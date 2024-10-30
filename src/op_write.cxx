#include "Logger.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  Logger::log("entry: ", __FILE__, ":", __LINE__);
  OFile *fp = (OFile *)fi->fh;  // 有个问题, 这里的 path 没有用到
  if (fp == nullptr) {
    return -ENOENT;
  }

  if (offset > fp->ip->dinode.size) {
    return -1;  // 不允许出现文件空洞
  }

  int ret = fp->write(buf, size);

  Logger::log("entry: ", __FILE__, ":", __LINE__);
  return ret;
}
