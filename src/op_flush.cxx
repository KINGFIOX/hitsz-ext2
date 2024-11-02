#include "INodeCache.h"
#include "Log.h"
#include "Logger.h"
#include "OFile.h"
#include "common.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_flush(const char *path, struct fuse_file_info *fi) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  // inode
  for (auto kv : INodeCache::inodes) {
    Log::begin_op();
    INode *ip = kv.second;
    ip->lock();
    ip->update();
    ip->unlock();
    Log::end_op();
  }

  // 我确保了每次都是 log_write 的, 因此, 按道理来说, 做了修改的 block, 都在 log(mem) 中有记录
  // 我只需要将 log(mem) 中的内容写入 log(disk) 即可
  Log::write_log();   // Write modified blocks from cache to log
  Log::write_head();  // Write header to disk -- the real commit

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}