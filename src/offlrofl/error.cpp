#include <offlrofl/error.h>

#include <memory>
#include <stdexcept>

extern "C" {
#include <dbus/dbus.h>
}

namespace offlrofl {
error::error() {
  err = std::make_unique<DBusError>();

  dbus_error_init(*this);
}

error::~error() {
  if (err != nullptr) {
    dbus_error_free(*this);
  }
}

auto error::is_ok() const -> bool {
  return !is_error();
}

auto error::is_error() const -> bool {
  return dbus_error_is_set(*this) != 0;
}

void error::throw_if_error() const {
  if (is_error()) {
    throw std::runtime_error(message());
  }
}

auto error::message() const noexcept -> const char* {
  return err->message != nullptr ? err->message : "";
}

error::operator DBusError*() {
  return err.get();
}

error::operator const DBusError*() const {
  return err.get();
}
}
