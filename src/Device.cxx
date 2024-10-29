#include "Device.h"

#include <unistd.h>

#include "common.h"

Device dev_table[NDEV];

int Console::read(uint64_t dst, uint32_t n) {
  std::lock_guard<std::mutex> lock(Console::mtx);
  ::read(0, (char*)dst, n);
  return 0;
}

int Console::write(uint64_t src, uint32_t n) {
  std::lock_guard<std::mutex> lock(Console::mtx);
  ::write(1, (const char*)src, n);
  return 0;
}

void Console::init(void) {
  dev_table[CONSOLE].read = Console::read;
  dev_table[CONSOLE].write = Console::write;
}