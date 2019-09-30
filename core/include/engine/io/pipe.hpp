#pragma once

/// @file engine/io/pipe.hpp
/// @brief @copybrief engine::io::Pipe

#include <engine/deadline.hpp>
#include <engine/io/common.hpp>
#include <engine/io/exception.hpp>
#include <engine/io/fd_control_holder.hpp>
#include <utils/clang_format_workarounds.hpp>

namespace engine::io {

namespace impl {
class FdControl;
}  // namespace impl

/// Reading end of an unidirectional pipe
class PipeReader final {
 public:
  /// Whether the reading end of the pipe is valid.
  bool IsValid() const;

  /// Suspends current task until the pipe has data available.
  bool WaitReadable(Deadline);

  /// Receives at least one bytes from the pipe.
  [[nodiscard]] size_t ReadSome(void* buf, size_t len, Deadline deadline);

  /// Receives exactly len bytes from the pipe.
  /// @note Can return less than len if pipe is closed by peer.
  [[nodiscard]] size_t ReadAll(void* buf, size_t len, Deadline deadline);

  /// File descriptor corresponding to the read end of the pipe.
  int Fd() const;

  /// Releases reading end file descriptor and invalidates it.
  [[nodiscard]] int Release() noexcept;

  /// Closes and invalidates the reading end of the pipe.
  /// @warn You should not call Close with pending I/O. This may work okay
  /// sometimes but it's loosely predictable.
  void Close();

 private:
  friend class Pipe;

  PipeReader() = default;
  explicit PipeReader(int fd);

  impl::FdControlHolder fd_control_;
};

/// Writing end of an unidirectional pipe
class PipeWriter final {
 public:
  /// Whether the writing end of the pipe is valid.
  bool IsValid() const;

  /// Suspends current task until the pipe can accept more data.
  bool WaitWriteable(Deadline);

  /// Sends exactly len bytes to the pipe.
  /// @note Can return less than len if pipe is closed by peer.
  [[nodiscard]] size_t WriteAll(const void* buf, size_t len, Deadline deadline);

  /// File descriptor corresponding to the write end of the pipe.
  int Fd() const;

  /// Releases writing end file descriptor and invalidates it.
  [[nodiscard]] int Release() noexcept;

  /// Closes and invalidates the writing end of the pipe.
  /// @warn You should not call Close with pending I/O. This may work okay
  /// sometimes but it's loosely predictable.
  void Close();

 private:
  friend class Pipe;

  PipeWriter() = default;
  explicit PipeWriter(int fd);

  impl::FdControlHolder fd_control_;
};

/// Unidirectional pipe representation
class Pipe final {
 public:
  /// Constructs a unidirectional pipe
  Pipe();

  PipeReader reader;
  PipeWriter writer;
};

}  // namespace engine::io