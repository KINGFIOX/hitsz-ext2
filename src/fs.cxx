#include <sys/types.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "Block.h"
#include "BlockCache.h"
#include "DirEntry.h"
#include "DiskINode.h"
#include "INode.h"
#include "INodeCache.h"
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

uint32_t INode::read(uint64_t dst, off_t off, uint32_t n) {
  if (off > this->dinode.size || off + n < off) return 0;
  if (off + n > this->dinode.size) n = this->dinode.size - off;  // return type should be same as this->dinode.size

  uint32_t tot, m;
  for (tot = 0; tot < n; tot += m, off += m, dst += m) {
    uint32_t bno_phy = block_map(this, off / BSIZE);
    if (bno_phy == 0) break;
    Block *bp = BlockCache::block_read(this->dev, bno_phy);
    m = min(n - tot, BSIZE - off % BSIZE);

    ::memmove((char *)dst, bp->data + (off % BSIZE), m);
    BlockCache::block_release(bp);
  }
  return tot;
}

uint32_t INode::write(uint64_t src, off_t off, uint32_t n) {
  if (off > this->dinode.size || off + n < off) return -1;
  if (off + n > MAX_FILE * BSIZE) return -1;

  uint32_t tot, m;
  for (tot = 0; tot < n; tot += m, off += m, src += m) {
    uint32_t bno_phy = block_map(this, off / BSIZE);
    if (bno_phy == 0) break;
    Block *bp = BlockCache ::block_read(this->dev, bno_phy);
    m = min(n - tot, BSIZE - off % BSIZE);
    ::memmove(bp->data + (off % BSIZE), (char *)src, m);
    Log::log_write(bp);
    BlockCache::block_release(bp);
  }

  if (off > this->dinode.size) this->dinode.size = off;

  // write the i-node back to disk even if the size didn't change
  // because the loop above might have called bmap() and added a new
  // block to ip->addrs[].
  this->update();

  return tot;
}

INode *INode::dirlookup(const char *name, off_t *poff) {
  if (this->dinode.type != T_DIR) assert(0 && "dirlookup not DIR");

  for (off_t off = 0; off < this->dinode.size; off += sizeof(DirEntry)) {
    DirEntry de;
    if (this->read((uint64_t)&de, off, sizeof(de)) != sizeof(de)) assert(0 && "dirlockup read");
    if (de.inum == 0) continue;
    if (strcmp(name, de.name) == 0) {
      // entry matches path element
      if (poff) *poff = off;
      uint32_t inum = de.inum;
      return INodeCache::inode_get(this->dev, inum);
    }
  }

  return 0;
}

int INode::dirlink(char *name, uint32_t inum) {
  // Check that name is not present.
  INode *ip = this->dirlookup(name, 0);
  if (ip != 0) {
    INodeCache::inode_put(ip);
    return -1;
  }

  // Look for an empty dirent.
  off_t off;
  DirEntry de;
  for (off = 0; off < this->dinode.size; off += sizeof(de)) {
    if (this->read((uint64_t)&de, off, sizeof(de)) != sizeof(de)) assert(0 && "dirlink read");
    if (de.inum == 0) break;
  }

  strncpy(de.name, name, DIRSIZ);
  de.inum = inum;
  if (this->write((uint64_t)&de, off, sizeof(de)) != sizeof(de)) return -1;

  return 0;
}

/// Paths
///
/// @brief Copy the next path element from path into name.
/// @return Return a pointer to the element following the copied one.
/// The returned path has no leading slashes, so the caller can check *path=='\0' to see if the name is the last one. If no name to remove, return 0.
///
/// @param ret (mut)
///
/// Examples:
///   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
///   skipelem("///a//bb", name) = "bb", setting name = "a"
///   skipelem("a", name) = "", setting name = "a"
///   skipelem("", name) = skipelem("////", name) = 0
///
///
static const char *skipelem(const char *path, char *ret) {
  while (*path == '/') path++;
  if (*path == 0) return 0;
  const char *s = path;
  while (*path != '/' && *path != 0) path++;
  int len = path - s;
  if (len >= DIRSIZ)
    memmove(ret, s, DIRSIZ);
  else {
    memmove(ret, s, len);
    ret[len] = 0;
  }
  while (*path == '/') path++;
  return path;
}

/// @brief Look up and return the inode for a path name.
/// If parent != 0, return the inode for the parent and copy the final path element into name, which must have room for DIRSIZ bytes.
/// @warning Must be called inside a transaction since it calls inode_put().
[[maybe_unused]] static INode *namex(const char *path, int nameiparent, char *name) {
  INode *ip;
  if (*path == '/') {
    ip = INodeCache::inode_get(ROOTDEV, ROOTINO);
  } else {
    // ip = idup(myproc()->cwd);
  }

  // skipelem("a/bb/c", name) = "bb/c", setting name = "a"
  // skipelem("///a//bb", name) = "bb", setting name = "a"
  // skipelem("a", name) = "", setting name = "a"
  // skipelem("", name) = skipelem("////", name) = 0
  while ((path = skipelem(path, name)) != 0) {
    ip->lock();
    if (ip->dinode.type != T_DIR) {
      INodeCache::inode_unlock_put(ip);
      return 0;
    }
    if (nameiparent && *path == '\0') {
      // Stop one level early.
      ip->unlock();
      return ip;
    }
    INode *next = ip->dirlookup(name, nullptr);
    if (next == 0) {
      INodeCache::inode_unlock_put(ip);
      return 0;
    }
    INodeCache::inode_unlock_put(ip);
    ip = next;
  }

  if (nameiparent) {
    INodeCache::inode_put(ip);
    return 0;
  }
  return ip;
}
