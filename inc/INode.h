#pragma once

#include <sys/types.h>

#include <cstdint>
#include <mutex>

#include "DiskINode.h"
#include "Stat.h"

struct INodeKey {
  uint32_t dev;
  uint32_t inum;

  INodeKey(uint32_t dev, uint32_t inum) : dev(dev), inum(inum) {}

  bool operator<(const INodeKey &other) const {
    if (dev != other.dev) return dev < other.dev;
    return inum < other.inum;
  }
};

struct INode {
  uint32_t dev;    // Device number
  uint32_t inum;   // Inode number
  uint32_t ref;    // Reference count
  std::mutex mtx;  // protects everything below here
  bool valid;      // inode has been read from disk?

  DiskINode dinode;  // copy of disk inode

 public:
  /// @brief Lock the given inode. Reads the inode from disk if necessary.
  void lock(void);

  /// @brief Unlock the inode.
  void unlock(void);

  /// @brief Truncate inode (discard contents).
  /// @warning Caller must hold ip->lock.
  void trunc(void);

  /// @brief Copy a modified in-memory inode to disk.
  /// @warning Must be called after every change to an ip->xxx field that lives on disk.
  /// @warning Caller must hold ip->lock.
  void update(void);

  /// @brief Copy stat information from inode.
  /// @warning Caller must hold ip->lock.
  void stat(Stat *st);

  /// @brief Read data from inode.
  /// @warning Caller must hold ip->lock.
  uint32_t read(uint64_t dst, off_t off, uint32_t n);

  /// @brief Write data to inode.
  /// @warning Caller must hold ip->lock.
  /// @return the number of bytes successfully written.
  /// @return If the return value is less than the requested n, there was an error of some kind.
  uint32_t write(uint64_t src, off_t off, uint32_t n);

  /// @brief Look for a directory entry in a directory.
  /// @param poff
  /// @return If found, set *poff to byte offset of entry.
  INode *dirlookup(const char *name, off_t *poff);

  /// @brief Write a new directory entry (name, inum) into the directory dp.
  /// @return Returns 0 on success, -1 on failure (e.g. out of disk blocks).
  int dirlink(char *name, uint32_t inum);
};