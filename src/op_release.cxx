#include "FileTable.h"
#include "Logger.h"
#include "OFile.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_release(const char *path, struct fuse_file_info *fi) {
  Logger::log("entry: ", __FILE__, ":", __LINE__);
  OFile *fp = (OFile *)fi->fh;  // 有个问题, 这里的 path 没有用到

  if (fp == nullptr) {
    return -ENOENT;
  }

  FileTable::ofile_close(fp);

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}
