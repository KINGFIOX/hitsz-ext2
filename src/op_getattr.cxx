#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "BlockCache.h"
#include "Device.h"
#include "DirEntry.h"
#include "DiskINode.h"
#include "INode.h"
#include "INodeCache.h"
#include "Log.h"
#include "Logger.h"
#include "OFile.h"
#include "PosixEnv.h"
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
  memset(st, 0, sizeof(*st));
  st->st_dev = ino->dev;
  st->st_ino = ino->inum;
  if (ino->dinode.type == DiskINode::T_DIR) {
    st->st_mode = S_IFREG | 0777;
  } else if (ino->dinode.type == DiskINode::T_FILE) {
    st->st_mode = S_IFDIR | 0777;
  } else {
    Logger::log("unknown type ", __FILE__, __LINE__, __FUNCTION__);
    Logger::log("unknown type ", __FILE__, __LINE__, __FUNCTION__);
  }
  st->st_nlink = ino->dinode.nlink;
  ::printf("        nlinks=%lu\n", st->st_nlink);
  st->st_uid = ::getuid();
  ::printf("        uid=%d\n", st->st_uid);
  st->st_gid = ::getgid();
  ::printf("        gid=%d\n", st->st_gid);
  st->st_rdev = 0;  // 不会是 char, block

  st->st_size = ino->dinode.size;
  st->st_blksize = BSIZE;
  st->st_blocks = (ino->dinode.size + BSIZE - 1) / BSIZE;
  st->st_atime = 0;
  st->st_mtime = 0;
  st->st_ctime = 0;
}

int op_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
  Logger::log("entry: ", __FILE__, ":", __LINE__);
  OFile *file = (OFile *)fi->fh;
  INode *ip = file->ip;
  do_fillstatbuf(ip, stbuf);
  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}

int op_getattr(const char *path, struct stat *stbuf) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);
  Logger::log("path = ", path);

  Log::begin_op();

  INode *ip = inode_name(path);

  if (ip == nullptr) {
    Logger::log("do_readinode(", path, ", &ino, &vnode); failed");
    return -ENOENT;
  }

  ip->lock();

  // ::printf("        nlinks=%d\n", ip->dinode.nlink);

  if (ip->dinode.type == DiskINode::T_DEVICE && (ip->dinode.major < 0 || ip->dinode.major >= NDEV)) {
    INodeCache::inode_unlock_put(ip);
    Log::end_op();
    return -EIO;
  }

  do_fillstatbuf(ip, stbuf);

  INodeCache::inode_unlock_put(ip);
  Log::end_op();

  // if (::strcmp(path, "/") == 0) {
  //   std::cout << "bno: " << ip->dinode.addrs[0] << std::endl;
  //   char buf[1024];
  //   ip->read((uint64_t)buf, 0, 5 * sizeof(DirEntry));
  //   std::cout << "bno: " << ip->dinode.addrs[0] << std::endl;
  //   for (int i = 0; i < 5; i++) {
  //     DirEntry *dir = (DirEntry *)buf;
  //     std::cout << "name: " << dir[i].name << "; " << "inum: " << dir[i].inum << std::endl;
  //   }
  // }

  Logger::log("path: ", path, ", size: ", stbuf->st_size);
  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}