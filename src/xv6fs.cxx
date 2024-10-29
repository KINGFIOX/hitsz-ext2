#include "xv6fs.h"

#include "config.h"

extern "C" {
#include <fuse.h>
}

void *xv6fs_init(struct fuse_conn_info *conn) {
  //   errcode_t rc;
  //   struct fuse_context *cntx = fuse_get_context();
  //   struct extfs_data *e2data = cntx->private_data;

  //   debugf("enter %s", e2data->device);

  //   rc = ext2fs_open(e2data->device, (e2data->readonly) ? 0 : EXT2_FLAG_RW, 0, 0, unix_io_manager, &e2data->e2fs);
  //   if (rc) {
  //     debugf("Error while trying to open %s", e2data->device);
  //     exit(1);
  //   }
  // #if 1
  //   if (e2data->readonly != 1)
  // #endif
  //     rc = ext2fs_read_bitmaps(e2data->e2fs);
  //   if (rc) {
  //     debugf("Error while reading bitmaps");
  //     ext2fs_close(e2data->e2fs);
  //     exit(1);
  //   }
  //   debugf("FileSystem %s", (e2data->e2fs->flags & EXT2_FLAG_RW) ? "Read&Write" : "ReadOnly");

  //   debugf("leave");

  // return e2data;
  return nullptr;
}