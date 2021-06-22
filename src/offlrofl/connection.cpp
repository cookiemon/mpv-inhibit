#include <offlrofl/connection.h>
#include <offlrofl/error.h>
#include <offlrofl/message.h>

#include <cassert>

extern "C" {
#include <dbus/dbus.h>
}

namespace offlrofl {
auto connection::session() -> connection {
  error err;
  dbus_error_init(err);

  DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, err);
  err.throw_if_error();

  return connection{conn};
}

auto connection::system() -> connection {
  error err;
  dbus_error_init(err);

  DBusConnection* conn = dbus_bus_get(DBUS_BUS_SYSTEM, err);
  err.throw_if_error();

  return connection{conn};
}

connection::connection(connection&& other) noexcept : conn{other.conn} {
  other.conn = nullptr;
}

auto connection::operator=(connection&& other) noexcept -> connection& {
  std::swap(conn, other.conn);

  return *this;
}

connection::~connection() {
  if (conn != nullptr) {
    dbus_connection_unref(conn);
  }
}

auto connection::send_with_reply(DBusMessage* msg) -> message {
  error err;
  auto* reply = dbus_connection_send_with_reply_and_block(*this, msg, -1, err);
  err.throw_if_error();

  return message::wrap(reply);
}

connection::operator DBusConnection*() {
  return conn;
}

connection::connection(DBusConnection* initConn) : conn{initConn} {
  assert(conn);
}
}
