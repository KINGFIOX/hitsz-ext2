#include <sys/types.h>

#include "FileTable.h"
#include "Logger.h"
#include "config.h"
#include "xv6fs.h"

extern "C" {
#include <fuse.h>
}

int op_chown(const char* path, uid_t uid, gid_t gid) {
  Logger::log("enter: ", __FILE__, ":", __LINE__);

  OFile* file = do_open(path, O_RDWR);
  if (file == NULL) {
    Logger::log("do_open failed", path);
    return -ENOENT;
  }

  FileTable::ofile_close(file);

  Logger::log("enter: ", __FILE__, ":", __LINE__);
  return 0;
}