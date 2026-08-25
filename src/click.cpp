#include "click.hpp"
#include "common.hpp"
#include "logger.hpp"
#include "x11.hpp"
extern Logger *lg;
extern X11 *x11inst;
namespace {
constexpr auto CLICK_DELAY_MS = 1;
}

ClickEngine::ClickEngine(PresetConfig config, QObject *parent)
    : m_preset(config), m_reps(m_preset.repeat), QObject(parent)
{
  connect(&m_timer, &QTimer::timeout, this, &ClickEngine::click);
}

ClickEngine::~ClickEngine() {
  stop();
}

void ClickEngine::start() {
  if (running) return;
  running = true;
  QTimer::singleShot(0, this, &ClickEngine::click);
}

void ClickEngine::click() {
  if (!running) return;
  nativeMouseClick(m_preset.location, m_preset.currentLocation, m_preset.mouseButton);

  if (!m_preset.repeatUntilStopped && --m_reps <= 0) {
    stop();
    emit clickFinished();
    return;
  }

  m_timer.start(m_preset.interval);
}

void ClickEngine::stop() {
  if (!running) return;
  running = false;
  m_timer.stop();
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
static DWORD to_win32(MouseButton button, bool isPress) {
  switch (button) {
  case MouseButton::Right: return isPress ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
  case MouseButton::Middle: return isPress ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
  default: return isPress ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
  }
}

} // namespace

void ClickEngine::nativeMouseClick(Location loc, bool current_loc, MouseButton mouseButton) const {
  int x = loc.x, y = loc.y;
  if (current_loc) {
    QPoint p = QCursor::pos();
    x = p.x(); y = p.y();
  }

  SetCursorPos(x, y);

  INPUT input{.type=INPUT_MOUSE, .mi={x, y}};

  // Down event
  input.mi.dwFlags = to_win32(mouseButton, true) | MOUSEEVENTF_ABSOLUTE;
  SendInput(1, &input, sizeof(INPUT));

  Sleep(CLICK_DELAY_MS);

  // Up event
  input.mi.dwFlags = to_win32(mouseButton, false) | MOUSEEVENTF_ABSOLUTE;
  SendInput(1, &input, sizeof(INPUT));
}

#else // X11
#include <unistd.h>
#include <xcb/xtest.h>

namespace {
int to_xcb(MouseButton mouseButton) {
  switch (mouseButton) {
  case MouseButton::Right: return 3;
  case MouseButton::Middle: return 2;
  default: return 1;
  }
}
} // namespace

void ClickEngine::nativeMouseClick(Location loc, bool current_loc, MouseButton mouseButton) const {
  int msbtnId = to_xcb(mouseButton);

  int x = loc.x, y = loc.y;

  if (!current_loc) {
    xcb_test_fake_input(x11inst->connection, XCB_MOTION_NOTIFY, 0,
                        XCB_CURRENT_TIME, x11inst->rootWindow, x, y, 0);
  } else {
    QPoint p = QCursor::pos();
    x = p.x(); y = p.y();
  }

  xcb_test_fake_input(x11inst->connection, XCB_BUTTON_PRESS, msbtnId,
                      XCB_CURRENT_TIME, x11inst->rootWindow, x, y, 0);

  usleep(CLICK_DELAY_MS * 1000);

  xcb_test_fake_input(x11inst->connection, XCB_BUTTON_RELEASE, msbtnId,
                      XCB_CURRENT_TIME, x11inst->rootWindow, x, y, 0);

  xcb_flush(x11inst->connection);
}

#endif
