#pragma once

#include <cstdint>
#include <mutex>

#include "OFile.h"

class FileTable {
 public:
  static inline std::mutex mtx;
  static OFile files[NFILE];

  /// @brief Allocate a file structure.
  /// Just alloc a slot in the file table. with cleared content
  static OFile* ofile_alloc(void);

  static OFile* ofile_dup(OFile* f);

  static void ofile_close(OFile* f);
};