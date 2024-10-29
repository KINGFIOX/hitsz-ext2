#include "config.h"

extern "C" {
#include <fuse.h>
}

#include <cstdlib>
#include <iostream>

#include "limits.h"

static const struct fuse_operations xv6fs_ops = {
    //     .getattr = op_getattr,
    //     .readlink = op_readlink,
    //     .mknod = op_mknod,
    //     .mkdir = op_mkdir,
    //     .unlink = op_unlink,
    //     .rmdir = op_rmdir,
    //     .symlink = op_symlink,
    //     .rename = op_rename,
    //     .link = op_link,
    //     .chmod = op_chmod,
    //     .chown = op_chown,
    //     .truncate = op_truncate,
    //     .open = op_open,
    //     .read = op_read,
    //     .write = op_write,
    //     .statfs = op_statfs,
    //     .flush = op_flush,
    //     .release = op_release,
    //     .fsync = op_fsync,
    //     .setxattr = NULL,
    //     .getxattr = op_getxattr,
    //     .listxattr = NULL,
    //     .removexattr = NULL,
    //     .opendir = op_open,
    //     .readdir = op_readdir,
    //     .releasedir = op_release,
    //     .fsyncdir = op_fsync,
    //     .init = op_init,
    //     .destroy = op_destroy,
    //     .access = op_access,
    //     .create = op_create,
    //     .ftruncate = op_ftruncate,
    //     .fgetattr = op_fgetattr,
    //     .lock = NULL,
    //     .utimens = op_utimens,
    //     .bmap = NULL,
};

// /// ./xv6fs log.txt ./fs.img ./mnt
int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << "<./log.txt> <./fs.img> <./mnt>" << std::endl;
    return 1;
  }
  //   char* log = argv[1];
  //   char* img = argv[2];
  //   char* mnt = argv[3];
  fuse_main(argc, argv, &xv6fs_ops, nullptr);
  return 0;
}
