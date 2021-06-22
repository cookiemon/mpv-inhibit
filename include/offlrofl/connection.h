#pragma once

#include "message.h"

struct DBusConnection;
struct DBusMessage;

namespace offlrofl {
/**
 * Wrapper class around CBusConnection.
 */
class connection {
public:
  /**
   * Return a connection to the session bus.
   */
  static auto session() -> connection;
  /**
   * Return a connection to the system bus.
   */
  static auto system() -> connection;

  connection(connection& other) = delete;
  auto operator=(connection& other) -> connection& = delete;

  connection(connection&& other) noexcept;
  auto operator=(connection&& other) noexcept -> connection&;

  ~connection();

  /**
   * Send a message over the dbus and block until a reply was received.
   */
  auto send_with_reply(DBusMessage* msg) -> message;

  operator DBusConnection*();

private:
  explicit connection(DBusConnection* initConn);

  DBusConnection* conn = nullptr;
};
}
