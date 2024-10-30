#include "Logger.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_access(const char *path, int mask) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);
  Logger::log("path = %s, mask = 0%o", path, mask);

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}