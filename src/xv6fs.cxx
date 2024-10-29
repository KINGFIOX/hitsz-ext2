#include "xv6fs.h"

#include "BlockCache.h"
#include "INodeCache.h"
#include "Log.h"
#include "Logger.h"
#include "PosixEnv.h"
#include "SuperBlock.h"
#include "common.h"
#include "config.h"

extern "C" {
#include <fuse.h>
}

void *op_init(struct fuse_conn_info *_) {
  // init IO
  struct fuse_context *cntx = fuse_get_context();
  XV6FSData *user_data = (XV6FSData *)cntx->private_data;
  PosixEnv::_mmap_base = user_data->_mmap_base;

  BlockCache::init();

  INodeCache::init();

  SuperBlock::init(ROOTDEV);

  Log::init(ROOTDEV);  // log_init should behind superblock_init

  Logger::log(__func__, "fuse init");

  // return e2data;
  return user_data;
}

void op_destroy(void *userdata) { Logger::log(__func__, "fuse destroy"); }