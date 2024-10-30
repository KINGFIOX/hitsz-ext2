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
#include "PosixEnv.h"
#include "SuperBlock.h"
#include "common.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

void *op_init(struct fuse_conn_info *_) {
  Logger::log("entry: ", __FILE__, ":", __LINE__);
  // init IO

  BlockCache::init();

  INodeCache::init();

  SuperBlock::init(ROOTDEV);

  Console::init();

  Log::init(ROOTDEV);  // log_init should behind superblock_init

  Logger::log(__func__, "fuse init");

  // return e2data;
  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return NULL;
}

void op_destroy(void *userdata) { Logger::log(__func__, "fuse destroy"); }