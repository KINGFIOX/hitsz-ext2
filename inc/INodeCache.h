#pragma once

#include <cstdint>
#include <mutex>

#include "INode.h"
#include "common.h"

class INodeCache {
 public:
  static std::mutex mtx;
  static INode inodes[NINODE];

 public:
  static void init(void);

  /// @brief return a slot of inode(with no lock) in inode cache
  static INode* inode_get(uint32_t dev, uint32_t inum);

  /// @brief Allocate an inode on device dev.  Mark it as allocated by  giving it type type.
  /// @return an unlocked but allocated and referenced inode, or NULL if there is no free inode.
  static INode* inode_alloc(uint32_t dev, uint16_t type);
};