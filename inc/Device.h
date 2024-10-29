#pragma once

#include <cstdint>

struct Device {
  int (*read)(int, uint64_t, int);
  int (*write)(int, uint64_t, int);
};

extern Device dev_table[];