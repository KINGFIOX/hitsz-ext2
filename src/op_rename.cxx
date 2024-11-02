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

static int same_parent(const std::filesystem::path &source, const std::filesystem::path &dest) {
  std::filesystem::path parent = source.parent_path();

  std::filesystem::path dest_base = dest.filename();
  std::filesystem::path source_base = source.filename();

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

static int different_parent(const std::filesystem::path &source, const std::filesystem::path &dest) {
  std::filesystem::path dest_parent = dest.parent_path();
  std::filesystem::path source_parent = source.parent_path();
  std::filesystem::path dest_base = dest.filename();
  std::filesystem::path source_base = source.filename();

  off_t off;
  uint32_t n;
  DirEntry de;
  int ret;

  Log::begin_op();

  INode *dp_src = inode_name(source_parent.c_str());
  if (dp_src == nullptr || dp_src->dinode.type != DiskINode::T_DIR) {
    Log::end_op();
    return -ENOENT;
  }

  INode *dp_dest = inode_name(dest_parent.c_str());
  if (dp_dest == nullptr || dp_dest->dinode.type != DiskINode::T_DIR) {
    INodeCache::inode_unlock_put(dp_src);
    Log::end_op();
    return -ENOENT;
  }

  INode *ip = dp_src->dirlookup(source_base.c_str(), &off);
  if (ip == nullptr) {
    INodeCache::inode_unlock_put(dp_src);
    INodeCache::inode_unlock_put(dp_dest);
    Log::end_op();
    return -ENOENT;
  }

  ret = dp_dest->dirlink(dest_base.c_str(), ip->inum);  // link to dest
  if (ret < 0) {
    INodeCache::inode_unlock_put(ip);
    INodeCache::inode_unlock_put(dp_src);
    INodeCache::inode_unlock_put(dp_dest);
    Log::end_op();
    return -EIO;
  }
  dp_dest->update();

  ::memset(&de, 0, sizeof(de));
  n = dp_src->read((uint64_t)&de, dp_src->dinode.size - sizeof(de), sizeof(de));
  if (n != sizeof(DirEntry)) {
    ::memset(&de, 0, sizeof(de));
    dp_dest->write((uint64_t)&de, dp_dest->dinode.size - sizeof(de), sizeof(de));  // roll back
    dp_dest->update();

    INodeCache::inode_unlock_put(ip);
    INodeCache::inode_unlock_put(dp_src);
    INodeCache::inode_unlock_put(dp_dest);
    Log::end_op();
    return -EIO;
  }
  dp_src->write((uint64_t)&de, off, sizeof(de));
  dp_src->update();

  INodeCache::inode_put(ip);
  INodeCache::inode_unlock_put(dp_src);
  INodeCache::inode_unlock_put(dp_dest);

  Log::end_op();

  return 0;
}

int op_rename(const char *source, const char *dest) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  int ret;
  std::filesystem::path source_path(source);
  std::filesystem::path dest_path(dest);

  if (source_path.parent_path() == dest_path.parent_path()) {
    ret = same_parent(source_path, dest_path);
  } else {
    ret = different_parent(source_path, dest_path);
  }

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return ret;
}