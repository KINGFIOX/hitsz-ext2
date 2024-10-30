#include "Logger.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_utimens(const char *path, const struct timespec tv[2]) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);
  // do nothing
  Logger::log("enter: ", __FILE__, ":", __LINE__);
  return 0;
}