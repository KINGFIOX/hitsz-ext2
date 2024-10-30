#include <cassert>
#include <cstdint>

#include "FileTable.h"
#include "Log.h"
#include "Logger.h"
#include "OFile.h"
#include "common.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_truncate(const char* path, off_t length) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  OFile* file = do_open(path, O_RDWR);
  if (file == NULL) {
    Logger::log("do_open failed", path);
    return -ENOENT;
  }

  assert(file->ip->dinode.size % BSIZE == 0);

  uint32_t n_block = file->ip->dinode.size / BSIZE;
  uint32_t n_block_new = (length + BSIZE - 1) / BSIZE;

  if (n_block_new < n_block) {
    // TODO
  }

  // 假装好像已经 truncate 了
  Log::begin_op();
  file->ip->lock();
  file->ip->dinode.size = n_block_new * BSIZE;
  file->ip->update();
  file->ip->unlock();
  Log::end_op();

  FileTable::ofile_close(file);

  Logger::log("enter: ", __FILE__, ":", __LINE__);
  return 0;
}