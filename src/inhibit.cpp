#include <screensaver_interface.h>

#include <fmt/format.h>
#include <mpv/client.h>

#include <string_view>

using namespace std::string_view_literals;

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
