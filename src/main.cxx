extern "C" {
#include <fuse.h>
}

#include <iostream>

int main(int argc, char** argv) {
  std::cout << "Hello, World!" << std::endl;
  // Return success
  return 0;
}