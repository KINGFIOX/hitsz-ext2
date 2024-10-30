#include <cstdint>
#include <cstring>

#include "Block.h"
#include "BlockCache.h"
#include "DiskINode.h"
#include "Logger.h"
#include "SuperBlock.h"
#include "common.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_statfs(const char *path, struct statvfs *buf) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  ::memset(buf, 0, sizeof(struct statvfs));

  buf->f_bsize = BSIZE;
  buf->f_frsize = BSIZE;

  __fsfilcnt_t block_cnt = 0;

  for (size_t b = 0; b < SuperBlock::size; b += BIT_PER_BLOCK) {
    Block *bp = BlockCache::block_read(ROOTDEV, BIT_BLOCK(b));
    for (size_t bi = 0; bi < BIT_PER_BLOCK && b + bi < SuperBlock::size; bi++) {
      size_t m = 1 << (bi % 8);  // mask
      if ((bp->data[bi / 8] & m) != 0) {
        block_cnt++;
      }
    }
    BlockCache::block_release(bp);
  }

  buf->f_blocks = FSSIZE;
  buf->f_bfree = FSSIZE - block_cnt;
  buf->f_bavail = FSSIZE - block_cnt;

  __fsfilcnt_t inode_cnt = 0;

  for (uint32_t inum = 1; inum < SuperBlock::ninodes; inum++) {
    Block *bp = BlockCache::block_read(ROOTDEV, INODE_BLOCK(inum));
    DiskINode *dip = (DiskINode *)bp->data + inum % INODE_PER_BLOCK;
    if (dip->type != 0) {
      inode_cnt++;
    }
    BlockCache::block_release(bp);
  }
  buf->f_files = inode_cnt;
  buf->f_favail = NINODES - inode_cnt;
  buf->f_ffree = NINODES - inode_cnt;

  buf->f_namemax = DIRSIZ;

  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}
