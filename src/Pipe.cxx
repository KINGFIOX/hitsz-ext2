#include "Pipe.h"

#include <cstdint>

#include "FileTable.h"
#include "OFile.h"

int Pipe::alloc(OFile **wr, OFile **rd) {
  *wr = *rd = nullptr;
  *wr = FileTable::ofile_alloc();
  if (*wr == nullptr) return -1;
  *rd = FileTable::ofile_alloc();
  if (*rd == nullptr) {
    FileTable::ofile_close(*wr);  // roll back
    *wr = nullptr;
    return -1;
  }
  Pipe *pi = new Pipe;
  if (pi == nullptr) {
    FileTable::ofile_close(*wr);
    *wr = nullptr;
    FileTable::ofile_close(*rd);
    *rd = nullptr;
    return -1;
  }
  (*wr)->type = OFile::FD_PIPE;
  (*wr)->readable = false;
  (*wr)->writable = true;
  (*wr)->pipe = pi;

  (*rd)->type = OFile::FD_PIPE;
  (*rd)->readable = true;
  (*rd)->writable = false;
  (*rd)->pipe = pi;

  return 0;
}

void Pipe::close(bool writable) {
  std::unique_lock<std::mutex> lk_grd(mtx);
  if (writable) {
    writeopen = false;
    cv_wr.notify_all();
  } else {
    readopen = false;
    cv_rd.notify_all();
  }
  if (!readopen && !writeopen) {
    lk_grd.unlock();
    delete this;
  }
}

int Pipe::write(const char *addr, uint32_t n) {
  uint32_t i = 0;

  std::unique_lock<std::mutex> lk_grd(mtx);
  while (i < n) {
    if (!this->readopen) {
      this->mtx.unlock();
      return -1;
    }
    if (this->nwrite == this->nread + PIPESIZE) {  // full
      cv_rd.notify_all();
      cv_wr.wait(lk_grd);
    } else {
      char ch = addr[i++];
      this->data[this->nwrite++ % PIPESIZE] = ch;
    }
  }

  this->cv_rd.notify_all();
  lk_grd.unlock();

  return i;
}

int Pipe::read(char *addr, uint32_t n) {
  uint32_t i = 0;

  std::unique_lock<std::mutex> lk_grd(mtx);
  while (i < n) {
    if (this->nwrite == this->nread && this->writeopen) {  // empty but has producer
      this->cv_rd.wait(lk_grd);
    }
    for (i = 0; i < n; i++) {
      if (this->nread == this->nwrite) break;  // empty, break to outer while
      char ch = this->data[this->nread++ % PIPESIZE];
      addr[i] = ch;
    }
  }

  this->cv_wr.notify_all();
  lk_grd.unlock();

  return i;
}