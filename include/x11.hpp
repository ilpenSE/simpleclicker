#pragma once
class X11;

#ifdef __linux__

#include <xcb/xcb.h>
#include <QApplication>

class X11 {
public:
  static X11& instance() {
    static X11 inst;
    return inst;
  }
  xcb_connection_t *connection = nullptr;
  xcb_window_t rootWindow{};

private:

  X11() {
    auto x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();
    connection = x11App->connection();

    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    rootWindow = screen->root;
  }

  ~X11() {}
};

#endif // __linux__
