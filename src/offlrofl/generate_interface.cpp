#include <offlrofl/connection.h>
#include <offlrofl/message.h>

#include <fmt/format.h>
#include <pugixml.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>

constexpr auto preamble = R"(
#pragma once

#include <offlrofl/connection.h>
#include <offlrofl/message.h>

#include <cstdint>
#include <string>

// Generated from {name}
)";

constexpr auto class_template = R"(
class {class} {{
public:
  {class}() = default;
  {class}(const char* init_destination, const char* init_path)
      : destination{{init_destination}}, path{{init_path}} {{}}

{methods}

  [[nodiscard]] auto get_destination() const -> const char* {{ return destination; }}
  [[nodiscard]] auto get_path() const -> const char* {{ return path; }}
  [[nodiscard]] static auto get_interface() -> const char* {{ return iface; }}

private:
  offlrofl::connection conn = offlrofl::connection::session();

  const char* destination = "{destination}";
  const char* path = "{path}";
  static inline const char* iface = "{interface}";

  template <typename ReturnType, typename... Args>
  auto call(const char* name, Args... args) -> ReturnType {{
    // The allocated character array is only valid as long as the
    // message is allocated which gets unreferenced at the end of this
    // method. So strings must be copied and returned instead.
    static_assert(!std::is_same_v<std::remove_cv_t<ReturnType>, const char*> &&
                      !std::is_same_v<std::remove_cv_t<ReturnType>, char*>,
                  "Returning strings as pointer to const is not supported. Use "
                  "std::string instead.");

    // Call function
    offlrofl::message msg = offlrofl::message::method_call(
        get_destination(), get_path(), get_interface(), name, args...);

    // Get return value
    auto reply = conn.send_with_reply(msg);
    if constexpr (std::is_same_v<std::remove_cv_t<ReturnType>, std::string>) {{
      // if the return type is a string we must extract the data through
      // a pointer to char.
      return reply.get_argument<const char*>();
    }} else {{
      return reply.get_argument<ReturnType>();
    }}
  }}
}};
)";

using namespace std::string_view_literals;

class org_freedesktop_DBus_Introspectable {
public:
  org_freedesktop_DBus_Introspectable() = default;
  org_freedesktop_DBus_Introspectable(const char* init_destination,
                                      const char* init_path)
      : destination{init_destination}, path{init_path} {}

  auto Introspect() -> std::string { return call<std::string>("Introspect"); }

  [[nodiscard]] auto get_destination() const -> const char* {
    return destination;
  }
  [[nodiscard]] auto get_path() const -> const char* { return path; }
  [[nodiscard]] static auto get_interface() -> const char* { return iface; }

private:
  offlrofl::connection conn = offlrofl::connection::session();

  const char* destination = "org.freedesktop.ScreenSaver";
  const char* path = "/org/freedesktop/ScreenSaver";
  static inline const char* const iface = "org.freedesktop.DBus.Introspectable";

  template <typename ReturnType, typename... Args>
  auto call(const char* name, Args... args) -> ReturnType {
    // The allocated character array is only valid as long as the
    // message is allocated which gets unreferenced at the end of this
    // method. So strings must be copied and returned instead.
    static_assert(!std::is_same_v<std::remove_cv_t<ReturnType>, const char*> &&
                      !std::is_same_v<std::remove_cv_t<ReturnType>, char*>,
                  "Returning strings as pointer to const is not supported. Use "
                  "std::string instead.");

    // Call function
    offlrofl::message msg = offlrofl::message::method_call(
        get_destination(), get_path(), get_interface(), name, args...);

    // Get return value
    auto reply = conn.send_with_reply(msg);
    if constexpr (std::is_same_v<std::remove_cv_t<ReturnType>, std::string>) {
      // if the return type is a string we must extract the data through
      // a pointer to char.
      return reply.get_argument<const char*>();
    } else {
      return reply.get_argument<ReturnType>();
    }
  }
};

void replace(std::string& str, char needle, char with) {
  std::replace_if(
      std::begin(str), std::end(str), [needle](char c) { return c == needle; },
      with);
}

auto retrieve_introspect_xml(const std::string& destination,
                             const std::string& path) -> std::string {
  auto object =
      org_freedesktop_DBus_Introspectable{destination.c_str(), path.c_str()};

  return object.Introspect();
}

/**
 * Transform the single char literals of dbus argument types to c++
 * types.
 */
auto translate_arg_type(const std::string& arg) -> std::optional<std::string> {
  // 0 is invalid anyway so use that if arg looks wrong
  char type_char = arg.size() == 1 ? arg[0] : '\0';
  switch (type_char) {
  case 'y':
    return "uint8_t";
  case 'b':
    return "bool";
  case 'n':
    return "uint16_t";
  case 'q':
    return "int16_t";
  case 'u':
    return "uint32_t";
  case 'i':
    return "int32_t";
  case 's':
    return "std::string";
  default:
    return std::nullopt;
  }
}

/**
 * Generate code for the synchronous function call for the method
 * specified by the given xml node.
 */
auto generate_method_code(const pugi::xml_node& method) -> std::string {
  std::string return_type;
  std::string arguments;
  std::string typed_arguments;

  std::string method_name = method.attribute("name").value();
  for (auto arg : method.children("arg")) {
    std::string arg_name = arg.attribute("name").value();

    std::string arg_dbus_type = arg.attribute("type").value();
    auto arg_type = translate_arg_type(arg_dbus_type);
    if (!arg_type) {
      fmt::print(
          stderr,
          "Unknown argument type '{}' for method '{}' argument '{}'. Skipping "
          "method.\n",
          arg_dbus_type, method_name, arg_name);
      return fmt::format(
          "  // {method} skipped. Argument type {type} unknown.\n",
          fmt::arg("method", method_name), fmt::arg("type", arg_dbus_type));
    }

    const auto* arg_direction = arg.attribute("direction").value();
    if (arg_direction == "in"sv) {
      // Arguments will be a list appended after the name parameter so
      // it allways needs to be prepended with a komma.
      arguments.append(", ").append(arg_name);

      if (arg_type == "std::string") {
        // Strings must be passed as c strings to underlying api anyway,
        // so make function parameter a c string.
        arg_type = "const char* ";
      }
      if (!typed_arguments.empty()) {
        typed_arguments.append(", ");
      }
      typed_arguments.append(*arg_type).append(" ").append(arg_name);

    } else if (arg_direction == "out"sv) {
      if (!return_type.empty()) {
        fmt::print(stderr, "Found multiple return arguments in method '{}'",
                   method_name);
        return fmt::format("  // {method} skipped. Multiple return values.\n");
      }

      return_type = *arg_type;
    } else {
      fmt::print(stderr,
                 "Unknown argument direction '{}' for method '{}' "
                 "argument '{}'.",
                 arg_direction, method_name, arg_name);
      return fmt::format(
          "  // {method} skipped, unknown argument direction '{direction}'\n",
          fmt::arg("method", method_name),
          fmt::arg("direction", arg_direction));
    }
  }

  // If no return value was found, use void
  if (return_type.empty()) {
    return_type = "void";
  }

  // clang-format off
  return fmt::format(
      "  {return_type} {method}({typed_arguments}){{ return call<{return_type}>(\"{method}\"{arguments}); }}\n",
			fmt::arg("return_type", return_type),
			fmt::arg("method", method_name),
			fmt::arg("typed_arguments", typed_arguments),
			fmt::arg("arguments", arguments));
  // clang-format on
}

auto generate_source_code(const std::string& interface_description,
                          const std::string& destination,
                          const std::string& path) -> std::string {
  pugi::xml_document doc;
  pugi::xml_parse_result res = doc.load_string(interface_description.c_str());
  if (!res) {
    throw res;
  }

  std::string code = fmt::format(preamble, fmt::arg("name", destination));

  for (auto interface : doc.child("node").children("interface")) {
    std::string interface_name = interface.attribute("name").value();
    fmt::print(stderr, "Generating interface for {}\n", interface_name);

    std::string class_name = interface_name;
    replace(class_name, '.', '_');

    std::string methods;
    for (auto method : interface.children("method")) {
      methods.append(generate_method_code(method));
    }

    code += fmt::format(
        class_template, fmt::arg("class", class_name),
        fmt::arg("methods", methods), fmt::arg("destination", destination),
        fmt::arg("path", path), fmt::arg("interface", interface_name));
  }

  return code;
}

auto main(int argc, const char** argv) -> int {
  try {
    if (argc < 2) {
      const auto* name = argc < 1 ? "generate_interface" : argv[0];
      fmt::print(stderr,
                 "Usage: {} object-destination\n"
                 "object-destination may either be a path or a destination. "
                 "(Example: org.freedesktop.ScreenSaver)",
                 name);
      return EXIT_FAILURE;
    }

    std::string destination = argv[1];
    replace(destination, '/', '.');
    if (!destination.empty() && destination[0] == '.') {
      destination.erase(0, 1);
    }

    std::string path = argv[1];
    replace(path, '.', '/');
    if (!path.empty() && path[0] != '/') {
      path.insert(0, 1, '/');
    }

    auto xml = retrieve_introspect_xml(destination, path);
    auto code = generate_source_code(xml, destination, path);

    fmt::print("{}", code);

    return EXIT_SUCCESS;
  } catch (const std::exception& e) {
    fmt::print(stderr, "Unknown error: {}\n", e.what());
  } catch (const pugi::xml_parse_result& e) {
    fmt::print(stderr, "Xml parse error: {}\n", e.description());
  }
  return EXIT_FAILURE;
}
