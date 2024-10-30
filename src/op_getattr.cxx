#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "BlockCache.h"
#include "Device.h"
#include "DirEntry.h"
#include "DiskINode.h"
#include "FileTable.h"
#include "INode.h"
#include "INodeCache.h"
#include "Log.h"
#include "Logger.h"
#include "OFile.h"
#include "PosixEnv.h"
#include "Stat.h"
#include "SuperBlock.h"
#include "common.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

/// @param in (const)
/// @param st (mut)
void do_fillstatbuf(const INode *ino, struct stat *st) {
  ::memset(st, 0, sizeof(*st));
  // st->st_dev = ino->dev;  // ignored
  // st->st_blksize = BSIZE; // ignored

  st->st_ino = ino->inum;
  if (ino->dinode.type == DiskINode::T_DIR) {
    st->st_mode = S_IFDIR | ACCESSPERMS;
  } else if (ino->dinode.type == DiskINode::T_FILE) {
    st->st_mode = S_IFREG | ACCESSPERMS;
  } else {
    ::printf("unknown type: %d", ino->dinode.type);
    std::abort();
  }
  st->st_nlink = ino->dinode.nlink;

  // st->st_uid = ::getuid();  // ignored
  // st->st_gid = ::getgid();
  // st->st_rdev = 0;

  // st->st_atime = 0; // ignored
  // st->st_mtime = 0;
  // st->st_ctime = 0;

  st->st_size = ino->dinode.size;
  st->st_blocks = (ino->dinode.size + BSIZE - 1) / BSIZE;
}

int op_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
  Logger::log("entry: ", __FILE__, ":", __LINE__);
  OFile *file = (OFile *)fi->fh;
  if (file == nullptr) {
    return -ENOENT;
  }
  INode *ip = file->ip;
  do_fillstatbuf(ip, stbuf);
  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}

int op_getattr(const char *path, struct stat *stbuf) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  OFile *file = do_open(path, O_RDONLY);

  if (file == nullptr) {
    Logger::log("do_open(", path, "); failed");
    return -ENOENT;
  }

  do_fillstatbuf(file->ip, stbuf);

  FileTable::ofile_close(file);

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}