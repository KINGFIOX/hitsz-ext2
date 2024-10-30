#include <cstdint>
#include <cstring>

#include "Logger.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

/// @brief Get extended attributes.
/// @param path (const) file path
/// @param name (const) xattr name
/// @param buf (mut) buffer to store xattr value
/// @param size (const) buffer size
int op_getxattr(const char *path, const char *name, char *buf, size_t size) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  ::strncpy(buf, "NULL", size);

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}

int op_listxattr(const char *path, char *buf, size_t size) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  ::strncpy(buf, "NULL", size);

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}