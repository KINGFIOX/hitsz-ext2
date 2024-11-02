#include <fcntl.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>

#include "DirEntry.h"
#include "DiskINode.h"
#include "INode.h"
#include "INodeCache.h"
#include "Log.h"
#include "Logger.h"
#include "common.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

static int do_rename_same(const char *source, const char *dest) {
  std::filesystem::path source_path(source);
  std::filesystem::path dest_path(dest);
  std::filesystem::path parent = source_path.parent_path();

  std::filesystem::path dest_base = dest_path.filename();
  std::filesystem::path source_base = source_path.filename();

  Log::begin_op();
  INode *dp = inode_name(parent.c_str());
  if (dp == nullptr) {
    Log::end_op();
    return -ENOENT;
  }

  if (dp->dinode.type != DiskINode::T_DIR) {
    Log::end_op();
    return -ENOTDIR;
  }

  off_t off;
  INode *ip = dp->dirlookup(source_base.c_str(), &off);
  if (ip == nullptr) {
    INodeCache::inode_unlock_put(dp);
    Log::end_op();
    return -ENOENT;
  }

  DirEntry de;
  ::memset(&de, 0, sizeof(de));
  de.inum = ip->inum;
  ::strncpy((char *)&de.name, dest_base.c_str(), DIRSIZ);

  uint32_t n = dp->write((uint64_t)&de, off, sizeof(de));
  if (n != sizeof(DirEntry)) {
    INodeCache::inode_unlock_put(ip);
    INodeCache::inode_unlock_put(dp);
    Log::end_op();
    return -EIO;
  }

  dp->update();
  INodeCache::inode_unlock_put(ip);
  INodeCache::inode_unlock_put(dp);
  Log::end_op();

  return 0;
}

int op_rename(const char *source, const char *dest) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  int ret;
  std::filesystem::path source_path(source);
  std::filesystem::path dest_path(dest);

  if (source_path.parent_path() == dest_path.parent_path()) {
    ret = do_rename_same(source, dest);
  } else {
    ret = do_link(source, dest);
    if (ret != 0) {
      return ret;
    }
    ret = do_unlink(source);
  }

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return ret;
}