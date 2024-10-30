#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

// 定义linux_dirent64结构体
struct linux_dirent64 {
  __ino64_t d_ino;         /* 64-bit inode number */
  __off64_t d_off;         /* 64-bit offset to next structure */
  unsigned short d_reclen; /* Size of this dirent */
  unsigned char d_type;    /* File type */
  char d_name[];           /* Filename (null-terminated) */
};

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
  int fd;
  char *path;
  char buf[BUF_SIZE];
  int nread;
  struct linux_dirent64 *d;
  int bpos;

  // 如果有命令行参数，使用第一个参数作为路径，否则使用当前目录
  if (argc > 1) {
    path = argv[1];
  } else {
    path = ".";
  }

  // 打开目录，使用 O_RDONLY | O_DIRECTORY
  fd = open(path, O_RDONLY | O_DIRECTORY);
  if (fd == -1) {
    fprintf(stderr, "can't open: '%s': %s\n", path, strerror(errno));
    return EXIT_FAILURE;
  }

  // 读取目录内容
  while (1) {
    // 使用getdents64系统调用读取目录项
    nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE);
    if (nread == -1) {
      fprintf(stderr, "读取目录 '%s' 时出错: %s\n", path, strerror(errno));
      close(fd);
      return EXIT_FAILURE;
    }

    if (nread == 0) break;  // 读取完毕

    // 遍历缓冲区中的目录项
    for (bpos = 0; bpos < nread;) {
      d = (struct linux_dirent64 *)(buf + bpos);
      printf("%s\n", d->d_name);
      bpos += d->d_reclen;
    }
  }

  // 关闭目录文件描述符
  if (close(fd) == -1) {
    fprintf(stderr, "无法关闭目录 '%s': %s\n", path, strerror(errno));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}