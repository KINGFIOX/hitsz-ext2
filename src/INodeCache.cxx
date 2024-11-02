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

void INodeCache::init(void) {
  // do nothing
}

INode *INodeCache::inode_get(uint32_t dev, uint32_t inum) {
  mtx.lock();

  auto it = inodes.find(INodeKey(dev, inum));
  if (it != inodes.end()) {
    INode *ip = it->second;
    ip->ref++;
    mtx.unlock();
    return ip;
  }

  INode *ip = new INode();
  ip->dev = dev;
  ip->inum = inum;
  ip->ref = 1;
  ip->valid = false;
  inodes[INodeKey(dev, inum)] = ip;
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

INode *INodeCache::inode_dup(INode *ip) {
  assert(ip != nullptr && "ip == nullptr");
  mtx.lock();
  ip->ref++;
  mtx.unlock();
  return ip;
}

void INodeCache::inode_put(INode *ip) {
  mtx.lock();

  if (ip->ref == 1 && ip->valid && ip->dinode.nlink == 0) {
    // inode has no links and no other references: truncate and free.

    // ip->ref == 1 means no other process can have ip locked,
    // so this acquiresleep() won't block (or deadlock).
    ip->mtx.lock();

    mtx.unlock();

    ip->trunc();
    ip->dinode.type = 0;
    ip->update();  // update the disk
    ip->valid = false;

    ip->mtx.unlock();

    mtx.lock();
  }

  ip->ref--;
  if (ip->ref == 0) {
    inodes.erase(INodeKey(ip->dev, ip->inum));
    delete ip;
  }
  mtx.unlock();
}

void INodeCache::inode_unlock_put(INode *ip) {
  ip->unlock();
  inode_put(ip);
}
