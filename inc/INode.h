#pragma once

#include <cstdint>
#include <mutex>

#include "DiskINode.h"

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
};