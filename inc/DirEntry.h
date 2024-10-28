#pragma once

#include <cstdint>

#include "common.h"

struct DirEntry {
  uint32_t inum;
  char name[DIRSIZ];
};