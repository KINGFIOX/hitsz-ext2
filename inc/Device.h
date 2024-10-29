#pragma once

#include <cstdint>
#include <mutex>

struct Device {
  int (*read)(uint64_t dst, uint32_t n);
  int (*write)(uint64_t src, uint32_t n);
};

extern Device dev_table[];

class Console {
 public:
  static int read(uint64_t dst, uint32_t n);
  static int write(uint64_t src, uint32_t n);
  static inline std::mutex mtx;

  static void init(void);
};