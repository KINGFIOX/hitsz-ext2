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

int op_mkdir(const char *path, mode_t mode) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  Log::begin_op();
  INode *ip = create(path, DiskINode::T_DIR, 0, 0);
  INodeCache::inode_unlock_put(ip);
  Log::end_op();

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}