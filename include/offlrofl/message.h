#pragma once

#include <dbus/dbus.h>

#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace offlrofl {
/**
 * Represents messages sent over dbus.
 */
class message {
public:
  message(const message&) = delete;
  message& operator=(const message&) = delete;

  message(message&& other) noexcept;
  message& operator=(message&& other) noexcept;

  ~message();

  /**
   * Adopt a pre-existing dbus message. On destruction of the returned
   * object unref will be called without previously having called ref.
   * It is meant to be directly passed messages from dbus create
   * functions. If you additionally want to have a separate reference
   * you should call ref on the message before passing it to this
   * function.
   */
  [[nodiscard]] static message wrap(DBusMessage* msg);

  /**
   * Create a message that specifies a method call on an interface.
   */
  template <typename... Args>
  [[nodiscard]] static message method_call(const char* destination,
                                           const char* path,
                                           const char* iface,
                                           const char* method,
                                           Args&... args);

  /**
   * Returns the first argument of the message. Currently only supports
   * basic dbus types, i.e. only `intX_t` and `const char*`.
   */
  template <typename T>
  [[nodiscard]] T get_argument();

  operator DBusMessage*();

private:
  explicit message(DBusMessage* initMsg);

  DBusMessage* msg = nullptr;
};

// IMPLEMENTATION DETAILS, PLEASE CLOSE YOUR EYES!
////////////////////////////////////////////////////////////////////////
namespace detail {
inline int message_arg_type(const char* /*unused*/) {
  return DBUS_TYPE_STRING;
}

inline int message_arg_type(uint8_t /*unused*/) {
  return DBUS_TYPE_BYTE;
}

inline int message_arg_type(bool /*unused*/) {
  return DBUS_TYPE_BOOLEAN;
}

inline int message_arg_type(uint16_t /*unused*/) {
  return DBUS_TYPE_UINT16;
}

inline int message_arg_type(int16_t /*unused*/) {
  return DBUS_TYPE_INT16;
}

inline int message_arg_type(uint32_t /*unused*/) {
  return DBUS_TYPE_UINT32;
}

inline int message_arg_type(int32_t /*unused*/) {
  return DBUS_TYPE_INT32;
}

inline int message_arg_type(uint64_t /*unused*/) {
  return DBUS_TYPE_UINT64;
}

inline int message_arg_type(int64_t /*unused*/) {
  return DBUS_TYPE_INT64;
}

// Appends no arguments (recursion base case).
inline void append_arguments(DBusMessageIter& /*iter*/) {}

// Append arguments to a dbus message.
template <typename T, typename... Args>
inline void append_arguments(DBusMessageIter& iter,
                             T& first_arg,
                             Args&... args) {
  dbus_message_iter_append_basic(&iter, message_arg_type(first_arg),
                                 &first_arg);

  append_arguments(iter, args...);
}
}

template <typename... Args>
message message::method_call(const char* destination,
                             const char* path,
                             const char* iface,
                             const char* method,
                             Args&... args) {
  assert(destination);
  assert(path);
  assert(iface);
  assert(method);

  auto msg =
      message{dbus_message_new_method_call(destination, path, iface, method)};

  DBusMessageIter iter;
  dbus_message_iter_init_append(msg, &iter);
  detail::append_arguments(iter, args...);

  return msg;
}

template <typename T>
T message::get_argument() {
  DBusMessageIter iter;
  dbus_message_iter_init(*this, &iter);

  int type = dbus_message_iter_get_arg_type(&iter);
  if (type != detail::message_arg_type(T{})) {
    throw std::runtime_error("unexpected argument type");
  }

  T buffer;
  dbus_message_iter_get_basic(&iter, &buffer);
  return buffer;
}

// Overload of void s.t. messages without return values are handled
// correctly.
template <>
inline void message::get_argument<void>() {}
}
