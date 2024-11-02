#pragma once

#include <cstdint>
#include <map>
#include <mutex>

#include "INode.h"
#include "common.h"

class INodeCache {
 private:
  static inline std::mutex mtx;
  static inline std::map<INodeKey, INode*> inodes;

 public:
  static void init(void);

  /// @brief Find the inode with number inum on device dev
  /// @return the in-memory copy.
  /// @warning Does not lock the inode and does not read it from disk.
  static INode* inode_get(uint32_t dev, uint32_t inum);

  /// @brief Allocate an inode on device dev.  Mark it as allocated by  giving it type type.
  /// @return an unlocked but allocated and referenced inode, or NULL if there is no free inode.
  static INode* inode_alloc(uint32_t dev, uint16_t type);

  /// @brief this func only called in fork. 发生 fork 的时候, cwd(current working directory) 也会指向只当前的文件夹,
  /// 为了防止数据的不安全: inode cache 中对应的 slot 被偷梁换柱 的情况
  static INode* inode_dup(INode* ip);

  /// @brief Drop a reference to an in-memory inode.
  /// If that was the last reference, the inode cache slot can be recycled.
  /// If that was the last reference and the inode has no links to it, free the inode (and its content) on disk.
  /// @warning All calls to inode_put() must be inside a transaction in case it has to free the inode.
  static void inode_put(INode* ip);

  /// @brief Common idiom: unlock, then put.
  static void inode_unlock_put(INode* ip);
};