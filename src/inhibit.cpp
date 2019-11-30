#include <screensaver_interface.h>

#include <fmt/format.h>
#include <mpv/client.h>

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

static void unpause(org_freedesktop_ScreenSaver& screen_saver,
                    uint32_t& cookie) {
  // New state: unpaused, deactivate screensaver
  if (cookie == 0) {
    cookie = screen_saver.Inhibit("mpv", "playing movie");
  } else {
    fmt::print("Double inhibit request detected.\n");
  }
}

static void pause(org_freedesktop_ScreenSaver& screen_saver, uint32_t& cookie) {
  // New state: paused, reactivate screensaver
  if (cookie != 0) {
    screen_saver.UnInhibit(cookie);
    cookie = 0;
  } else {
    fmt::print("Double uninhibit request detected.\n");
  }
}

extern "C" {
int mpv_open_cplugin(mpv_handle* handle) {
  set_thread_name(os_text("mpv/inhibit"));

  org_freedesktop_ScreenSaver screen_saver;

  uint32_t cookie = 0;

  unpause(screen_saver, cookie);

  // Enter event loop
  //////////////////////////////////////////////////////////////////////
  while (true) {
    mpv_event* evt = mpv_wait_event(handle, -1);
    switch (evt->event_id) {
    case MPV_EVENT_SHUTDOWN:
      return 0;

    case MPV_EVENT_PAUSE:
      pause(screen_saver, cookie);
      break;
    case MPV_EVENT_UNPAUSE:
      unpause(screen_saver, cookie);
      break;

    default:
      break;
    }
  }

  return 0;
}
}
