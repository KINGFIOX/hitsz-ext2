#include "Logger.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_rename(const char *source, const char *dest) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);
  int ret;
  ret = do_link(source, dest);
  if (ret != 0) {
    return ret;
  }
  ret = do_unlink(source);
  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return ret;
}