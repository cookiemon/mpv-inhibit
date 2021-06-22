#include <screensaver_interface.h>

#include <fmt/format.h>
#include <mpv/client.h>

#include <cstdint>
#include <string_view>
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define os_text(x) (L##x)
using os_str_type = const wchar_t*;
#elif defined(__linux)
#include <pthread.h>
using os_str_type = const char*;
constexpr auto os_text(os_str_type x) -> os_str_type {
  return x;
}
#else
#include <pthread_np.h>
#define text(x) (x)
using os_str_type = const char*;
constexpr auto os_text(os_str_type x) -> os_str_type {
  return x;
}
#endif

static void set_thread_name(os_str_type name) {
#if defined(WIN32)
  SetThreadDescription(GetCurrentThread(), name);
#elif defined(__linux)
  pthread_setname_np(pthread_self(), name);
#else
  pthread_set_name_np(pthread_self(), name);
#endif
}

constexpr uint64_t L33T = 1337;

static void unpause(org_freedesktop_ScreenSaver& screen_saver,
                    uint32_t& cookie) {
  // New state: unpaused, deactivate screensaver
  if (cookie == 0) {
    cookie = screen_saver.Inhibit("mpv", "playing movie");
  }
}

static void pause(org_freedesktop_ScreenSaver& screen_saver, uint32_t& cookie) {
  // New state: paused, reactivate screensaver
  if (cookie != 0) {
    screen_saver.UnInhibit(cookie);
    cookie = 0;
  }
}

extern "C" {
auto mpv_open_cplugin(mpv_handle* handle) -> int {
  try {
    set_thread_name(os_text("mpv/inhibit"));

    org_freedesktop_ScreenSaver screen_saver;

    uint32_t cookie = 0;

    auto res = mpv_observe_property(handle, L33T, "pause", MPV_FORMAT_FLAG);
    if (res < 0) {
      fmt::print("Cannot register property observer. Error: {}",
                 mpv_error_string(res));
      return -1;
    }

    // Enter event loop
    //////////////////////////////////////////////////////////////////////
    while (true) {
      mpv_event* evt = mpv_wait_event(handle, -1);
      switch (evt->event_id) {
      case MPV_EVENT_SHUTDOWN:
        return 0;

      case MPV_EVENT_PROPERTY_CHANGE:
        // Should always be set but check just in case.
        if (evt->reply_userdata == L33T) {
          auto* data = static_cast<mpv_event_property*>(evt->data);
          // Should never be somethimg else but check just in case.
          if (data->format == MPV_FORMAT_FLAG && data->data != nullptr) {
            int flag = *static_cast<int*>(data->data);
            if (flag != 0) {
              unpause(screen_saver, cookie);
            } else {
              pause(screen_saver, cookie);
            }
          }
        }

      default:
        break;
      }
    }

    return 0;
  } catch (...) {
    return -1;
  }
}
}
