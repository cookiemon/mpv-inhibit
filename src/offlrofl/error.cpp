#include <offlrofl/error.h>

#include <stdexcept>

extern "C" {
#include <dbus/dbus.h>
}

namespace offlrofl {
error::error() {
  err = std::shared_ptr<DBusError>{new DBusError{}, [](DBusError* err) {
                                     // assumption: can free invalid but
                                     // initialized DBusError
                                     dbus_error_free(err);
                                     delete err;
                                   }};

  dbus_error_init(*this);
}

bool error::is_ok() const {
  return !is_error();
}

bool error::is_error() const {
  return dbus_error_is_set(*this) != 0;
}

void error::throw_if_error() const {
  if (is_error()) {
    throw std::runtime_error(message());
  }
}

const char* error::message() const noexcept {
  return err->message != nullptr ? err->message : "";
}

error::operator DBusError*() {
  return err.get();
}

error::operator const DBusError*() const {
  return err.get();
}
}
