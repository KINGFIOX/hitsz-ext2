#include <cerrno>

#include "Logger.h"
#include "OFile.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);
  Logger::log("path = %s, mode: 0%o", path, mode);

  OFile *fp = do_open(path, O_CREAT | mode);

  if (fp == nullptr) {
    Logger::log("op_open(path, fi); failed");
    return -EIO;
  }

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}