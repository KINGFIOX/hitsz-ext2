#pragma once

#define BSIZE 1024  // block size

#define MAX_OP_BLOCKS 10              // max # of blocks any FS op writes
#define LOG_SIZE (MAX_OP_BLOCKS * 3)  // max data blocks in on-disk log
#define NBUF (MAX_OP_BLOCKS * 3)      // size of disk block cache