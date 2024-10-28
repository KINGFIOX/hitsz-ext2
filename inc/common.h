#pragma once

// block

#define BSIZE 1024  // block size

// log

#define MAX_OP_BLOCKS 10              // max # of blocks any FS op writes
#define LOG_SIZE (MAX_OP_BLOCKS * 3)  // max data blocks in on-disk log
#define NBUF (MAX_OP_BLOCKS * 3)      // size of disk block cache

// direct index

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAX_FILE (NDIRECT + NINDIRECT)

// dir entry

#define DIRSIZ 14

// inode

#define ROOTINO 1  // root i-number

// fs

#define FSMAGIC 0x10203040

#define BIT_PER_BLOCK (BSIZE * 8)  // Bitmap bits per block

#define BIT_BLOCK(b) ((b) / BIT_PER_BLOCK + SuperBlock::bmapstart)  // b 在 bitmap 中的 block number