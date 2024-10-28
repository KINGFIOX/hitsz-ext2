#include "INode.h"

#include <cassert>
#include <cstring>

#include "Block.h"
#include "BlockCache.h"
#include "common.h"

void INode::unlock(void) {
  //   if (ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1) panic("iunlock");
  assert(this->ref >= 1 && "iunlock");

  this->mtx.unlock();
}

void INode::lock(void) {
  assert(this->ref >= 1 && "ilock");

  this->mtx.lock();

  if (this->valid == 0) {
    Block *bp = BlockCache::block_read(this->dev, INODE_BLOCK(this->inum));
    DiskINode *dip = (DiskINode *)bp->data + this->inum % INODE_PER_BLOCK;
    this->dinode.type = dip->type;
    this->dinode.major = dip->major;
    this->dinode.minor = dip->minor;
    this->dinode.nlink = dip->nlink;
    this->dinode.size = dip->size;
    ::memmove(this->dinode.addrs, dip->addrs, sizeof(this->dinode.addrs));
    BlockCache::block_release(bp);
    this->valid = 1;
    assert(this->dinode.type != 0 && "ilock: no type");
  }
}

void INode::stat(Stat *st) {
  st->dev = this->dev;
  st->ino = this->inum;
  st->type = this->dinode.type;
  st->nlink = this->dinode.nlink;
  st->size = this->dinode.size;
}
