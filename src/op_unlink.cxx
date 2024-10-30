#include <cassert>
#include <cstdint>
#include <cstring>

#include "DirEntry.h"
#include "DiskINode.h"
#include "INode.h"
#include "INodeCache.h"
#include "Log.h"
#include "Logger.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

static bool is_dir_empty(INode* dp) {
  DirEntry de;

  for (uint32_t off = 2 * sizeof(de); off < dp->dinode.size; off += sizeof(de)) {
    uint32_t ret = dp->read((uint64_t)&de, off, sizeof(de));
    assert(ret == sizeof(de) && "isdirempty: readi");
    if (de.inum != 0) return false;
  }
  return true;
}

int op_unlink(const char* path) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);
  Logger::log("path = ", path);

  char basename[DIRSIZ];

  Log::begin_op();
  INode* dp = inode_name_parent(path, basename);
  if (dp == nullptr) {
    Log::end_op();
    return -1;
  }

  if (::strcmp(basename, ".") == 0 || ::strcmp(basename, "..") == 0) {
    INodeCache::inode_unlock_put(dp);
    Log::end_op();
    return -1;
  }

  off_t off;
  INode* ip = dp->dirlookup(basename, &off);
  if (ip == nullptr) {
    INodeCache::inode_unlock_put(dp);
    Log::end_op();
    return -1;
  }

  ip->lock();
  assert(ip->dinode.nlink >= 1 && "unlink: nlink");

  if (ip->dinode.type == DiskINode::T_DIR && !is_dir_empty(ip)) {
    INodeCache::inode_unlock_put(ip);
    INodeCache::inode_unlock_put(dp);
    Log::end_op();
    return -1;
  }

  DirEntry de;
  ::memset(&de, 0, sizeof(de));
  uint32_t ret = dp->write((uint64_t)&de, off, sizeof(de));
  assert(ret == sizeof(de) && "unlink: writei");
  if (ip->dinode.type == DiskINode::T_DIR) {
    dp->dinode.nlink--;
    dp->update();
  }
  INodeCache::inode_unlock_put(dp);

  ip->dinode.nlink--;
  ip->update();
  INodeCache::inode_unlock_put(ip);

  Log::end_op();

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}