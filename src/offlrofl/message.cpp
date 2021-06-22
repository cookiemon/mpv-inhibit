#include <offlrofl/message.h>

#include <dbus/dbus.h>

namespace offlrofl {
message::message(message&& other) noexcept : msg{other.msg} {
  other.msg = nullptr;
}

auto message::operator=(message&& other) noexcept -> message& {
  std::swap(msg, other.msg);

  return *this;
}

message::~message() {
  if (msg != nullptr) {
    dbus_message_unref(msg);
  }
}

message::message(DBusMessage* initMsg) : msg{initMsg} {}

auto message::wrap(DBusMessage* msg) -> message {
  return message{msg};
}

message::operator DBusMessage*() {
  return msg;
}
}
