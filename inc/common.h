#pragma once

#define BSIZE 1024  // block size

#define MAXOPBLOCKS 10             // max # of blocks any FS op writes
#define LOGSIZE (MAXOPBLOCKS * 3)  // max data blocks in on-disk log
#define NBUF (MAXOPBLOCKS * 3)     // size of disk block cache