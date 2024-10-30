//! [ boot block | super block | log(log_hdr + n logs) | inode blocks | free bit map | data blocks]

#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>

#include "Block.h"
#include "common.h"

/// @brief log header(disk && mem)
struct LogHeader {
  uint32_t n;
  uint32_t block[LOG_SIZE];
};

class Log {
 public:
  static uint32_t start;
  static uint32_t size;
  static uint32_t outstanding;  // how many FS sys calls are executing.
  static bool committing;       // in commit(), please wait.
  static uint32_t dev;
  static LogHeader log_hdr;

  static void init(uint32_t dev);

  static void begin_op(void);

  static void end_op(void);

  /// @brief pin a block in the cache, because of the ref in log_hdr(mem)
  ///
  /// Caller has modified b->data and is done with the buffer.
  /// Record the block number and pin in the cache by increasing refcnt.
  /// commit()/write_log() will do the disk write.
  ///
  /// log_write() replaces bwrite(); a typical use is:
  ///   bp = bread(...)
  ///   modify bp->data[]
  ///   log_write(bp)
  ///   brelse(bp)
  static void log_write(Block *b);

 private:
  /// @brief LogHeader(disk) -> LogHeader(mem)
  static void read_head(void);

  /// @brief LogHeader(mem) -> LogHeader(disk)
  static void write_head(void);

  static void commit(void);

  static void recover_from_log(void);

  /// @brief block_cache(mem) -> log(disk)
  friend int op_flush(const char *path, struct fuse_file_info *fi);
  static void write_log(void);

  ///
  static std::mutex mtx;
  static std::condition_variable cv;
};
