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

  /**
   * Check whether no error happened.
   */
  [[nodiscard]] bool is_ok() const;
  /**
   * Check whether an error happened.
   */
  [[nodiscard]] bool is_error() const;

  /**
   * Throw this instance if it contains an error.
   */
  void throw_if_error() const;

  /**
   * Returns the error message (or empty string if no message is
   * available).
   */
  [[nodiscard]] const char* message() const noexcept;

  /**
   * Allow the error instance to be used for dbus functions.
   */
  operator DBusError*();

  /**
   * Allow the error instance to be used for dbus functions.
   */
  operator const DBusError*() const;

private:
  std::shared_ptr<DBusError> err;
};
}
