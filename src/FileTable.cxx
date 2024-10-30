#include "FileTable.h"

#include <cassert>
#include <cstring>

#include "INodeCache.h"
#include "Log.h"
#include "OFile.h"
#include "common.h"

OFile FileTable::files[NFILE];

OFile* FileTable::ofile_alloc(void) {
  std::lock_guard<std::mutex> lock(mtx);
  for (int i = 0; i < NFILE; i++) {
    if (files[i].type == OFile::FD_NONE) {
      files[i].ref = 1;
      return &files[i];
    }
  }
  return nullptr;
}

OFile* FileTable::ofile_dup(OFile* f) {
  std::lock_guard<std::mutex> lock(mtx);
  if (f->ref < 1) assert(0 && "ofile_dup");
  f->ref++;
  return f;
}

void FileTable::ofile_close(OFile* f) {
  mtx.lock();
  if (f->ref < 1) assert(0 && "ofile_close");
  if (--f->ref > 0) {
    mtx.unlock();
    return;
  }
  OFile ff = *f;  // 注意, 这个 f 是全局数组中的元素, 这个 ff 是栈上的局部变量
  f->ref = 0;
  f->type = OFile::FD_NONE;
  mtx.unlock();

  if (ff.type == OFile::FD_PIPE) {
    ff.pipe->close(ff.writable);
  } else if (ff.type == OFile::FD_INODE || ff.type == OFile::FD_DEVICE) {
    Log::begin_op();
    INodeCache::inode_put(ff.ip);
    Log::end_op();
  } else {
    assert(0 && "ofile_close");
  }
}
