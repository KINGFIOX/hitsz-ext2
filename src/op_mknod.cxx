#include <cstdio>

#include "Log.h"
#include "Logger.h"
#include "OFile.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_mknod(const char *path, mode_t mode, dev_t dev) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  Log::begin_op();

  ::printf("mknod: path = %s, mode = %x, dev = %ld\n", path, mode, dev);

  Log::end_op();

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}