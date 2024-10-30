#include <fcntl.h>

#include <cstddef>
#include <cstring>

#include "DirEntry.h"
#include "INode.h"
#include "INodeCache.h"
#include "Logger.h"
#include "OFile.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
  Logger::log("entry: ", __FILE__, ":", __LINE__);
  OFile *fp = (OFile *)fi->fh;
  if (fp == nullptr) {
    return -ENOENT;
  }
  if (offset > fp->ip->dinode.size) {
    return 0;
  }
  if (offset % sizeof(DirEntry) != 0) {
    return -EINVAL;
  }
  for (size_t i = offset; i < fp->ip->dinode.size; i += sizeof(DirEntry)) {
    DirEntry de;
    ::memset(&de, 0, sizeof(de));
    fp->read((char *)&de, sizeof(DirEntry));
    if (de.inum == 0) {
      continue;
    }
    struct stat st;
    INode *de_ip = INodeCache::inode_get(fp->ip->dev, de.inum);
    de_ip->lock();
    do_fillstatbuf(de_ip, &st);
    INodeCache::inode_unlock_put(de_ip);
    if (filler(buf, de.name, &st, i + sizeof(DirEntry)) != 0) {
      return -ENOMEM;
    }
  }
  Logger::log("leave: ", __FILE__, ":", __LINE__);
  return 0;
}