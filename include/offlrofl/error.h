#pragma once

#include <memory>

struct DBusError;

namespace offlrofl {
/**
 * Wrapper around DBusError.
 * @note Because of some design limitations it is implemented as a
 * reference type and passing copies to dbus functions will affect the
 * original instance (and other copies and vice-versa).
 */
class error {
public:
  /**
   * Initializes error with 'no error' value.
   */
  error();

  error(const error&) = delete;
  error(error&&) = default;
  auto operator=(const error&) -> error& = delete;
  auto operator=(error&&) -> error& = default;
  ~error();

  /**
   * Check whether no error happened.
   */
  [[nodiscard]] auto is_ok() const -> bool;
  /**
   * Check whether an error happened.
   */
  [[nodiscard]] auto is_error() const -> bool;

  /**
   * Throw this instance if it contains an error.
   */
  void throw_if_error() const;

  /**
   * Returns the error message (or empty string if no message is
   * available).
   */
  [[nodiscard]] auto message() const noexcept -> const char*;

  /**
   * Allow the error instance to be used for dbus functions.
   */
  operator DBusError*();

  /**
   * Allow the error instance to be used for dbus functions.
   */
  operator const DBusError*() const;

private:
  std::unique_ptr<DBusError> err;
};
}
