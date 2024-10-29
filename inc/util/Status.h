#pragma once

#include "Slice.h"

class Status {
 public:
  // Create a success status.
  Status() noexcept : state_(nullptr) {}
  ~Status() { delete[] state_; }

  Status(const Status &rhs);
  Status &operator=(const Status &rhs);

  Status(Status &&rhs) noexcept : state_(rhs.state_) { rhs.state_ = nullptr; }
  Status &operator=(Status &&rhs) noexcept;

  static Status OK() { return Status(); }                                                                         // Return a success status.
  static Status NotFound(const Slice &msg, const Slice &msg2 = Slice()) { return Status(kNotFound, msg, msg2); }  // Return error status of an appropriate type.
  static Status Corruption(const Slice &msg, const Slice &msg2 = Slice()) { return Status(kCorruption, msg, msg2); }
  static Status NotSupported(const Slice &msg, const Slice &msg2 = Slice()) { return Status(kNotSupported, msg, msg2); }
  static Status InvalidArgument(const Slice &msg, const Slice &msg2 = Slice()) { return Status(kInvalidArgument, msg, msg2); }
  static Status IOError(const Slice &msg, const Slice &msg2 = Slice()) { return Status(kIOError, msg, msg2); }

  bool ok() const { return (state_ == nullptr); }                        // Returns true iff the status indicates success.
  bool IsNotFound() const { return code() == kNotFound; }                // Returns true iff the status indicates a NotFound error.
  bool IsCorruption() const { return code() == kCorruption; }            // Returns true iff the status indicates a Corruption error.
  bool IsIOError() const { return code() == kIOError; }                  // Returns true iff the status indicates an IOError.
  bool IsNotSupportedError() const { return code() == kNotSupported; }   // Returns true iff the status indicates a NotSupportedError.
  bool IsInvalidArgument() const { return code() == kInvalidArgument; }  // Returns true iff the status indicates an InvalidArgument.

  // Return a string representation of this status suitable for printing.
  // Returns the string "OK" for success.
  std::string ToString() const;

 private:
  enum Code {  // 可以用一个 char 来搞定
    kOk = 0,
    kNotFound = 1,
    kCorruption = 2,
    kNotSupported = 3,
    kInvalidArgument = 4,
    kIOError = 5
  };

  Code code() const { return (state_ == nullptr) ? kOk : static_cast<Code>(state_[4]); }

  Status(Code code, const Slice &msg, const Slice &msg2);

  static const char *CopyState(const char *s);

  // OK status has a null state_.  Otherwise, state_ is a new[] array
  // of the following form:
  //    state_[0..3] == length of message
  //    state_[4]    == code
  //    state_[5..]  == message
  const char *state_;
};