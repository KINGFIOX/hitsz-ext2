#include <fcntl.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>

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

/// @warning return ip locked
INode *create(const char *path, uint16_t type, uint16_t major, uint16_t minor) {
  char name[DIRSIZ];

  assert(type == DiskINode::T_FILE || type == DiskINode::T_DIR);

  INode *dp = inode_name_parent(path, name);
  if (!dp) return 0;

  dp->lock();

  INode *ip = dp->dirlookup(name, 0);
  if (ip) {
    INodeCache::inode_unlock_put(dp);
    ip->lock();
    // 如果原本存在, 并且 type 也符合, 则返回
    if (type == DiskINode::T_FILE && (ip->dinode.type == DiskINode::T_FILE || ip->dinode.type == DiskINode::T_DEVICE)) return ip;
    INodeCache::inode_unlock_put(ip);
    return nullptr;  // conflict
  }

  // if not exist, create one
  ip = INodeCache::inode_alloc(dp->dev, type);
  if (!ip) {
    INodeCache::inode_unlock_put(dp);
    return nullptr;
  }

  ip->lock();  // return ip locked
  ip->dinode.major = major;
  ip->dinode.minor = minor;
  ip->dinode.nlink = 1;
  ip->update();

  if (type == DiskINode::T_DIR) {  // Create . and .. entries.
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if (ip->dirlink(".", ip->inum) < 0 || ip->dirlink("..", dp->inum) < 0) {
      ip->dinode.nlink = 0;  // clean up
      ip->update();
      INodeCache::inode_unlock_put(ip);
      INodeCache::inode_unlock_put(dp);
      return nullptr;
    }
  }

  if (dp->dirlink(name, ip->inum) < 0) {
    ip->dinode.nlink = 0;  // clean up
    ip->update();
    INodeCache::inode_unlock_put(ip);
    INodeCache::inode_unlock_put(dp);
    return nullptr;
  }

  if (type == DiskINode::T_DIR) {
    // now that success is guaranteed:
    dp->dinode.nlink++;  // for ".."
    dp->update();
  }

  INodeCache::inode_unlock_put(dp);

  return ip;
}

OFile *do_open(const char *path, int omode) {
  Log::begin_op();

  INode *ip;
  if (omode & O_CREAT) {
    ip = create(path, DiskINode::T_FILE, 0, 0);
    if (!ip) {
      Log::end_op();
      return nullptr;
    }
  } else {
    ip = inode_name(path);
    if (!ip) {
      Log::end_op();
      return nullptr;
    }
    ip->lock();
    if (ip->dinode.type == DiskINode::T_DIR && ((omode & O_ACCMODE) != O_RDONLY) && (omode & O_TRUNC) && !(omode & O_DIRECTORY)) {
      INodeCache::inode_unlock_put(ip);
      Log::end_op();
      return nullptr;
    }
  }

  // if (ip->dinode.type == DiskINode::T_DEVICE && (ip->dinode.major < 0 || ip->dinode.major >= NDEV)) {
  //   INodeCache::inode_unlock_put(ip);
  //   Log::end_op();
  //   return nullptr;
  // }

  OFile *f = FileTable::ofile_alloc();
  if (!f) {
    INodeCache::inode_unlock_put(ip);
    Log::end_op();
    return nullptr;
  }

  if (ip->dinode.type == DiskINode::T_DEVICE) {
    f->type = OFile::FD_DEVICE;
    f->major = ip->dinode.major;
  } else {
    f->type = OFile::FD_INODE;
    f->off = 0;
  }
  f->ip = ip;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  if ((omode & O_TRUNC) && ip->dinode.type == DiskINode::T_FILE) {
    ip->trunc();
  }

  ip->unlock();
  Log::end_op();

  return f;
}

int op_open(const char *path, struct fuse_file_info *fi) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);
  Logger::log("path = ", path);

  OFile *file = do_open(path, fi->flags);
  if (file == nullptr) {
    Logger::log("do_open(", path, "); failed");
    return -ENOENT;
  }
  fi->fh = (uint64_t)file;
  Logger::log("fi->fh = ", fi->fh);

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}