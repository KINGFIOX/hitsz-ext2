#pragma once

class PosixEnv {
 private:
  friend void *xv6fs_init(struct fuse_conn_info *conn);
  static inline char *_mmap_base;
};