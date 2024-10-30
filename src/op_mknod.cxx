#include "Logger.h"
#include "OFile.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_mknod(const char *path, mode_t mode, dev_t dev) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  OFile *fp = do_open(path, O_CREAT | mode);

  if (fp == nullptr) {
    Logger::log("op_open(path, fi); failed");
    return -EIO;
  }

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}