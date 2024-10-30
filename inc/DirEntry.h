#pragma once

#include <cstdint>

#include "common.h"

struct DirEntry {
  uint16_t inum;
  char name[DIRSIZ];
};