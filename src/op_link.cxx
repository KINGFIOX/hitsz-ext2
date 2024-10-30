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

int do_link(const char *source, const char *dest) {
  char name[DIRSIZ];

  Log::begin_op();
  INode *ip = inode_name(source);
  if (!ip) {
    Log::end_op();
    return -1;
  }

  ip->lock();
  if (ip->dinode.type == DiskINode::T_DIR) {
    INodeCache::inode_unlock_put(ip);
    Log::end_op();
    return -1;
  }

  ip->dinode.nlink++;
  ip->unlock();
  ip->unlock();

  INode *dp = inode_name_parent(dest, name);
  if (!dp) {
    ip->lock();
    ip->dinode.nlink--;
    ip->update();
    INodeCache::inode_unlock_put(ip);
    Log::end_op();
    return -1;
  }
  dp->lock();
  if (dp->dev != ip->dev || dp->dirlink(name, ip->inum) < 0) {
    INodeCache::inode_unlock_put(dp);
    ip->lock();
    ip->dinode.nlink--;
    ip->update();
    INodeCache::inode_unlock_put(ip);
    Log::end_op();
    return -1;
  }
  INodeCache::inode_unlock_put(dp);
  INodeCache::inode_put(ip);

  Log::end_op();

  return 0;
}

int op_link(const char *source, const char *dest) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);
  int ret = do_link(source, dest);
  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return ret;
}