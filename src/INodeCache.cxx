#include "INodeCache.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "Block.h"
#include "BlockCache.h"
#include "DiskINode.h"
#include "INode.h"
#include "Log.h"
#include "SuperBlock.h"
#include "common.h"

std::mutex INodeCache::mtx;
INode INodeCache::inodes[NINODE];

void INodeCache::init(void) { /* do nothing */ }

INode *INodeCache::inode_get(uint32_t dev, uint32_t inum) {
  INode *ip = nullptr;
  INode *empty = nullptr;

  mtx.lock();

  // Is the inode already in the table?
  for (ip = &inodes[0]; ip < &inodes[NINODE]; ip++) {
    if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
      ip->ref++;
      mtx.unlock();
      return ip;
    }
    if (empty == 0 && ip->ref == 0)  // Remember empty slot.
      empty = ip;
  }

  // Recycle an inode entry.
  assert(empty && "no inodes");

  ip = empty;
  ip->dev = dev;
  ip->inum = inum;
  ip->ref = 1;
  ip->valid = 0;
  mtx.unlock();

  return ip;
}

INode *INodeCache::inode_alloc(uint32_t dev, uint16_t type) {
  for (uint32_t inum = 1; inum < SuperBlock::ninodes; inum++) {
    Block *bp = BlockCache::block_read(dev, INODE_BLOCK(inum));
    DiskINode *dip = (DiskINode *)bp->data + inum % INODE_PER_BLOCK;
    if (dip->type == 0) {  // a free inode
      ::memset(dip, 0, sizeof(*dip));
      dip->type = type;
      Log::log_write(bp);  // mark it allocated on the disk
      BlockCache::block_release(bp);
      return inode_get(dev, inum);
    }
    BlockCache::block_release(bp);
  }
  ::printf("ialloc: no inodes\n");
  return 0;
}