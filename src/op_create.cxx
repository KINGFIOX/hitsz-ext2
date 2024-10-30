#include <cerrno>
#include <cstdint>

#include "DiskINode.h"
#include "FileTable.h"
#include "INode.h"
#include "INodeCache.h"
#include "Log.h"
#include "Logger.h"
#include "OFile.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_create(const char *path, mode_t omode, struct fuse_file_info *fi) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  Log::begin_op();

  INode *ip = create(path, DiskINode::T_FILE, 0, 0);
  if (!ip) {
    Log::end_op();
    return -EIO;
  }

  OFile *fp = FileTable::ofile_alloc();
  if (!fp) {
    INodeCache::inode_unlock_put(ip);
    Log::end_op();
    return -EIO;
  }
  fp->type = OFile::FD_INODE;
  fp->off = 0;
  fp->ip = ip;

  // FIXME 这个 mode 这里, 有待商榷
  fp->readable = !(omode & O_WRONLY);
  fp->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  if ((omode & O_TRUNC) && ip->dinode.type == DiskINode::T_FILE) {
    ip->trunc();
  }

  ip->unlock();
  Log::end_op();

  fi->fh = (uint64_t)fp;

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}