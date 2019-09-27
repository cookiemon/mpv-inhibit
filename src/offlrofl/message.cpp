#include <offlrofl/message.h>

#include <dbus/dbus.h>

namespace offlrofl {
message::message(message&& other) noexcept : msg{other.msg} {
  other.msg = nullptr;
}

message& message::operator=(message&& other) noexcept {
  std::swap(msg, other.msg);

  return *this;
}

message::~message() {
  if (msg != nullptr) {
    dbus_message_unref(msg);
  }
}

message::message(DBusMessage* initMsg) : msg{initMsg} {}

message message::wrap(DBusMessage* msg) {
  return message{msg};
}

message::operator DBusMessage*() {
  return msg;
}
}
