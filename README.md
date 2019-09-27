# mpv-inhibit
Implements screensaver inhibit via systemd.

# Why?
mpv calls `xdg-screensaver reset` every few seconds to circumvent the
screensaver showing up. For me (and apparently others) this doesn't
work, so I wrote a fix.

# Build and install
 0. Run `git clone https://github.com/Cookiemon/mpv-inhibit.git`
 1. Install dev packages for dbus, pugixml, fmt and mpv. On Fedora do
    `sudo dnf install dbus-devel pugixml-devel fmt-devel mpv-devel`
 2. Install cmake. On Fedora do `sudo dnf install cmake`
 3. From the root of the cloned git run `cmake -Bbuild -DCMAKE_BUILD_TYPE=Release .`
 4. From the root of the cloned git run `cmake --build build`
 5. From the root of the cloned git run `mkdir -p ~/.config/mpv/scripts && cp build/libmpv-inhibit.so ~/.config/mpv/scripts`
