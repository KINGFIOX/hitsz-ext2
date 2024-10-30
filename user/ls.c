#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  DIR *dir;
  struct dirent *entry;
  char *path;

  // 如果有命令行参数，使用第一个参数作为路径，否则使用当前目录
  if (argc > 1) {
    path = argv[1];
  } else {
    path = ".";
  }

  // 打开目录
  dir = opendir(path);
  if (dir == NULL) {
    fprintf(stdout, "could not open dir '%s': %s\n", path, strerror(errno));
    return EXIT_FAILURE;
  }

  // 读取并打印目录内容
  while ((entry = readdir(dir)) != NULL) {
    // 忽略隐藏文件（以.开头的文件），如果需要显示所有文件可以删除下面这行
    // if (entry->d_name[0] == '.') continue;
    printf("%s\n", entry->d_name);
  }

  // 关闭目录
  if (closedir(dir) == -1) {
    fprintf(stdout, "could not close dir '%s': %s\n", path, strerror(errno));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
