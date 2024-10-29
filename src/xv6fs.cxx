#include "xv6fs.h"

#include <unistd.h>

#include <cstdint>
#include <cstring>

#include "BlockCache.h"
#include "DirEntry.h"
#include "DiskINode.h"
#include "INode.h"
#include "INodeCache.h"
#include "Log.h"
#include "Logger.h"
#include "PosixEnv.h"
#include "SuperBlock.h"
#include "common.h"
#include "config.h"

extern "C" {
#include <fuse.h>
}

/// @param in (const)
/// @param st (mut)
static void do_fillstatbuf(const INode *in, struct stat *st) {
  memset(st, 0, sizeof(*st));
  st->st_dev = in->dev;
  st->st_ino = in->inum;
  if (in->dinode.type == T_DIR) {
    st->st_mode = S_IFREG | 0755;
  } else if (in->dinode.type == T_FILE) {
    st->st_mode = S_IFDIR | 0644;
  } else {
    Logger::log("unknown type ", __FILE__, __LINE__, __FUNCTION__);
    Logger::log("unknown type ", __FILE__, __LINE__, __FUNCTION__);
  }
  st->st_nlink = in->dinode.nlink;
  st->st_uid = ::getuid();
  st->st_gid = ::gettid();
  st->st_rdev = 0;  // 不会是 char, block

  st->st_size = in->dinode.size;
  st->st_blksize = BSIZE;
  st->st_blocks = (in->dinode.size + BSIZE - 1) / BSIZE;
  st->st_atime = 0;
  st->st_mtime = 0;
  st->st_ctime = 0;
}

void *op_init(struct fuse_conn_info *_) {
  // init IO
  struct fuse_context *cntx = fuse_get_context();
  XV6FSData *user_data = (XV6FSData *)cntx->private_data;
  PosixEnv::_mmap_base = user_data->_mmap_base;

  BlockCache::init();

  INodeCache::init();

  SuperBlock::init(ROOTDEV);

  Log::init(ROOTDEV);  // log_init should behind superblock_init

  Logger::log(__func__, "fuse init");

  // return e2data;
  return user_data;
}

void op_destroy(void *userdata) { Logger::log(__func__, "fuse destroy"); }

int op_getattr(const char *path, struct stat *stbuf) {
  Logger::log("enter ", "path = ", path);

  INode *in = inode_name(path);
  if (::strcmp(path, "/") == 0) {
    std::cout << "bno: " << in->dinode.addrs[0] << std::endl;
    char buf[1024];
    in->read((uint64_t)buf, 0, 5 * sizeof(DirEntry));
    std::cout << "bno: " << in->dinode.addrs[0] << std::endl;
    for (int i = 0; i < 5; i++) {
      DirEntry *dir = (DirEntry *)buf;
      std::cout << "name: " << dir->name << "; " << "inum: " << dir->inum << std::endl;
    }
  }
  if (in == nullptr) {
    Logger::log("do_readinode(", path, ", &ino, &vnode); failed");
    return -ENOENT;
  }

  do_fillstatbuf(in, stbuf);

  Logger::log("path: %s, size: %d", path, stbuf->st_size);
  Logger::log("leave");
  return 0;
}