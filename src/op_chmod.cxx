#include "Logger.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_chmod(const char *path, mode_t mode) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);
  // do nothing
  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}