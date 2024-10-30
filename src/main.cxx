#include <fcntl.h>
#include <fuse/fuse_opt.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Logger.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

#include <cstdlib>
#include <iostream>

#include "limits.h"

#ifdef ENABLE_TESTS

#include <gtest/gtest.h>

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  return 0;
}

#else

static const struct fuse_operations xv6fs_ops = {
    .getattr = op_getattr,
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
    .open = op_open,
    .read = op_read,
    .write = op_write,
    .statfs = op_statfs,
    //     .flush = op_flush,
    .release = op_release,
    //     .fsync = op_fsync,
    .setxattr = NULL,
    .getxattr = op_getxattr,
    .listxattr = op_listxattr,
    .removexattr = NULL,
    .opendir = op_open,
    .readdir = op_readdir,
    .releasedir = op_release,
    //     .fsyncdir = op_fsync,
    .init = op_init,
    .destroy = op_destroy,
    .access = op_access,
    //     .create = op_create,
    //     .ftruncate = op_ftruncate,
    .fgetattr = op_fgetattr,
    .lock = NULL,
    //     .utimens = op_utimens,
    //     .bmap = NULL,
};

// /// ./xv6fs log.txt ./fs.img ./mnt
int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << "<./xv6fs.log> <./fs.img> <./mnt>" << std::endl;
    return 1;
  }

  char* img = argv[2];

  int fd = ::open(img, O_RDWR | O_CREAT, 0666);

  struct stat sb;
  if (::fstat(fd, &sb) == -1) {
    std::cerr << "fstat failed" << std::endl;
    std::abort();
  }

  char* _mmap_base = (char*)::mmap(nullptr, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  try {
    Logger::init(argv[1]);
    Logger::log("xv6fs version: ", VERSION);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    std::abort();
  }

  char* mnt = ::realpath(argv[3], nullptr);
  Logger::log("mount point: ", mnt);

  XV6FSData user_data = {.fd = fd, ._mmap_base = _mmap_base};

  argv[1] = "-f";
  argv[2] = "-d";
  argv[3] = mnt;

  ::fuse_main(argc, argv, &xv6fs_ops, &user_data);

  ::munmap(_mmap_base, sb.st_size);
  ::close(fd);

  ::free(mnt);
  mnt = nullptr;

  Logger::destroy();

  return 0;
}

#endif
