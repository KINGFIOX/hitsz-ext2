#pragma once

/// @brief this is the data that should pass to fuse_main

#include "INode.h"
#include "OFile.h"
#include "config.h"

extern "C" {
#include <fuse/fuse.h>
}

INode* inode_name(const char* path);
INode* inode_name_parent(const char* path, char* name);

void* op_init(struct fuse_conn_info* conn);
int op_getattr(const char* path, struct stat* stbuf);
int op_fgetattr(const char* path, struct stat* stbuf, struct fuse_file_info* fi);
int op_statfs(const char* path, struct statvfs* buf);
int op_open(const char* path, struct fuse_file_info* fi);
int op_access(const char* path, int mask);
void op_destroy(void* userdata);
int op_create(const char* path, mode_t mode, struct fuse_file_info* fi);
int op_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
int op_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi);
int op_getxattr(const char* path, const char* name, char* buf, size_t size);
int op_listxattr(const char* path, char* buf, size_t size);
int op_release(const char* path, struct fuse_file_info* fi);
int op_mknod(const char* path, mode_t mode, dev_t dev);
int op_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
int op_unlink(const char* path);
int op_setxattr(const char*, const char*, const char*, size_t, int);
int op_flush(const char* path, struct fuse_file_info* fi);
int op_utimens(const char* path, const struct timespec tv[2]);
int op_truncate(const char* path, off_t length);

OFile* do_open(const char* path, int omode);
void do_fillstatbuf(const INode* ino, struct stat* st);
INode* create(const char* path, uint16_t type, uint16_t major, uint16_t minor);
