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
  static connection session();
  /**
   * Return a connection to the system bus.
   */
  static connection system();

  connection(connection& other) = delete;
  connection& operator=(connection& other) = delete;

  connection(connection&& other) noexcept;
  connection& operator=(connection&& other) noexcept;

  ~connection();

  /**
   * Send a message over the dbus and block until a reply was received.
   */
  message send_with_reply(DBusMessage* msg);

  operator DBusConnection*();

private:
  explicit connection(DBusConnection* initConn);

  DBusConnection* conn = nullptr;
};
}
