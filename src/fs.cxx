#include <cassert>
#include <cstddef>
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

/// @brief clear the block(disk)'s data
static void block_zeroed(uint32_t dev, uint32_t bno) {
  Block *bp = BlockCache::block_read(dev, bno);
  ::memset(bp->data, 0, BSIZE);
  Log::log_write(bp);
  BlockCache::block_release(bp);
}

/// @brief Allocate a zeroed disk block.
/// @return bno; 0 if out of disk space.
static uint32_t block_alloc(uint32_t dev) {
  for (size_t b = 0; b < SuperBlock::size; b += BIT_PER_BLOCK) {
    Block *bp = BlockCache::block_read(dev, BIT_BLOCK(b));
    for (size_t bi = 0; bi < BIT_PER_BLOCK && b + bi < SuperBlock::size; bi++) {
      size_t m = 1 << (bi % 8);           // mask
      if ((bp->data[bi / 8] & m) == 0) {  // Is block free?
        bp->data[bi / 8] |= m;            // Mark block in use.
        Log::log_write(bp);
        BlockCache::block_release(bp);
        block_zeroed(dev, b + bi);
        return b + bi;
      }
    }
    BlockCache::block_release(bp);
  }
  ::printf("balloc: out of blocks\n");
  return 0;
}

/// @brief Free a disk block. only set the bit to 0 in bitmap.
static void block_free(uint32_t dev, uint32_t b) {
  Block *bp = BlockCache::block_read(dev, BIT_BLOCK(b));
  uint32_t bi = b % BIT_PER_BLOCK;
  uint32_t m = 1 << (bi % 8);
  assert((bp->data[bi / 8] & m) && "freeing free block");
  bp->data[bi / 8] &= ~m;
  Log::log_write(bp);
  BlockCache::block_release(bp);
}

void INode::trunc(void) {
  for (size_t i = 0; i < NDIRECT; i++) {
    if (this->dinode.addrs[i]) {
      block_free(this->dev, this->dinode.addrs[i]);
      this->dinode.addrs[i] = 0;
    }
  }

  if (this->dinode.addrs[NDIRECT]) {
    Block *bp = BlockCache::block_read(this->dev, this->dinode.addrs[NDIRECT]);
    uint32_t *a = (uint32_t *)bp->data;
    for (size_t j = 0; j < NINDIRECT; j++) {
      if (a[j]) block_free(this->dev, a[j]);
    }
    BlockCache::block_release(bp);
    block_free(this->dev, this->dinode.addrs[NDIRECT]);
    this->dinode.addrs[NDIRECT] = 0;
  }

  this->dinode.size = 0;
  this->update();
}

void INode::update(void) {
  Block *bp = BlockCache::block_read(this->dev, INODE_BLOCK(this->inum));
  DiskINode *dip = (DiskINode *)bp->data + this->inum % INODE_PER_BLOCK;
  dip->type = this->dinode.type;
  dip->major = this->dinode.major;
  dip->minor = this->dinode.minor;
  dip->nlink = this->dinode.nlink;
  dip->size = this->dinode.size;
  ::memmove(dip->addrs, this->dinode.addrs, sizeof(this->dinode.addrs));
  Log::log_write(bp);
  BlockCache::block_release(bp);
}

/// @brief Inode content
///
/// The content (data) associated with each inode is stored in blocks on the disk.
/// The first NDIRECT block numbers are listed in ip->addrs[].
/// The next NINDIRECT blocks are listed in block ip->addrs[NDIRECT].
///
/// @return the disk block address of the nth block in inode ip. If there is no such block, bmap allocates one.
/// @return 0 if out of disk space.
static uint32_t block_map(INode *ip, uint32_t bno_logi) {
  if (bno_logi < NDIRECT) {
    uint32_t bno_phy = ip->dinode.addrs[bno_logi];
    if (bno_phy == 0) {
      bno_phy = block_alloc(ip->dev);
      if (bno_phy == 0) return 0;
      ip->dinode.addrs[bno_logi] = bno_phy;
    }
    return bno_phy;
  }
  bno_logi -= NDIRECT;

  if (bno_logi < NINDIRECT) {
    // Load indirect block, allocating if necessary.
    uint32_t bno_phy = ip->dinode.addrs[NDIRECT];
    if (bno_phy == 0) {
      bno_phy = block_alloc(ip->dev);
      if (bno_phy == 0) return 0;
      ip->dinode.addrs[NDIRECT] = bno_phy;
    }
    Block *bp = BlockCache::block_read(ip->dev, bno_phy);
    uint32_t *dir = (uint32_t *)bp->data;
    bno_phy = dir[bno_logi];
    if (bno_phy == 0) {
      bno_phy = block_alloc(ip->dev);
      if (bno_phy) {
        dir[bno_logi] = bno_phy;
        Log::log_write(bp);
      }
    }
    BlockCache::block_release(bp);
    return bno_phy;
  }

  assert(0 && "bmap: out of range");
}