#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

// int setxattr(const char *, const char *, const char *, size_t, int) {}