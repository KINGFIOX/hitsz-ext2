#pragma once

// block

#define BSIZE 1024  // block size

// log

#define MAX_OP_BLOCKS 10              // max # of blocks any FS op writes
#define LOG_SIZE (MAX_OP_BLOCKS * 3)  // max data blocks in on-disk log
#define NBUF (MAX_OP_BLOCKS * 3)      // size of disk block cache

// direct index

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint32_t))
#define MAX_FILE (NDIRECT + NINDIRECT)

// dir entry

#define DIRSIZ 14

// inode

#include "SuperBlock.h"

#define ROOTINO 1  // root i-number

#define NINODE 50  // maximum number of i-nodes in inode cache

#define INODE_PER_BLOCK (BSIZE / sizeof(DiskINode))

#define INODE_BLOCK(i) ((i) / INODE_PER_BLOCK + SuperBlock::inodestart)  // Block containing inode i

// fs

#define FS_MAGIC 0x10203040

#define BIT_PER_BLOCK (BSIZE * 8)  // Bitmap bits per block

#define BIT_BLOCK(b) ((b) / BIT_PER_BLOCK + SuperBlock::bmapstart)  // b 在 bitmap 中的 block number