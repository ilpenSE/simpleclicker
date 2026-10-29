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
    : QObject(parent), m_preset(config), m_reps(m_preset.repeat)
{
  connect(&m_timer, &QTimer::timeout, this, &ClickEngine::click);
  m_timer.setSingleShot(true);
  m_holdTimer.setSingleShot(true);
  connect(&m_holdTimer, &QTimer::timeout, this, &ClickEngine::release);
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

  m_loc = getEffectiveLocation();
  if (!m_preset.currentLocation) {
    nativeMouseHover(m_loc);
  }

  nativeMousePress(m_preset.mouseButton, m_loc);
  m_pressed = true;

  int delay = m_preset.clickMode == ClickMode::Hold ? m_preset.holdTime : CLICK_DELAY_MS;
  m_holdTimer.start(delay);
}

void ClickEngine::release() {
  if (!m_pressed) return;
  nativeMouseRelease(m_preset.mouseButton, m_loc);
  m_pressed = false;

  if (!running) return;

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
  m_holdTimer.stop();
  if (m_pressed) {
    nativeMouseRelease(m_preset.mouseButton, m_loc);
    m_pressed = false;
  }
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
static void set_mouse_button_win32(INPUT &input, MouseButton button, bool press) {
  switch (button) {
  case MouseButton::Button4: {
    input.mi.dwFlags = press ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
    input.mi.mouseData = XBUTTON1;
  } break;
  case MouseButton::Button5: {
    input.mi.dwFlags = press ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
    input.mi.mouseData = XBUTTON2;
  } break;
  case MouseButton::Right: {
    input.mi.dwFlags = press ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
  } break;
  case MouseButton::Middle: {
    input.mi.dwFlags = press ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
  } break;
  default: {
    input.mi.dwFlags = press ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
  } break;
  }
  input.mi.dwFlags |= MOUSEEVENTF_ABSOLUTE;
}

} // namespace

void ClickEngine::nativeMouseHover(Location loc) const {
  SetCursorPos(loc.x, loc.y);
}

void ClickEngine::nativeMousePress(MouseButton mouseButton, Location loc) const {
  INPUT input{.type=INPUT_MOUSE, .mi={loc.x, loc.y}};
  set_mouse_button_win32(input, mouseButton, true);
  SendInput(1, &input, sizeof(input));
}

void ClickEngine::nativeMouseRelease(MouseButton mouseButton, Location loc) const {
  INPUT input{.type=INPUT_MOUSE, .mi={loc.x, loc.y}};
  set_mouse_button_win32(input, mouseButton, false);
  SendInput(1, &input, sizeof(input));
}

#else // X11
#include <unistd.h>
#include <xcb/xtest.h>

namespace {
int to_xcb(MouseButton mouseButton) {
  switch (mouseButton) {
  case MouseButton::Button4: return 8;
  case MouseButton::Button5: return 9;
  case MouseButton::Right: return 3;
  case MouseButton::Middle: return 2;
  default: return 1;
  }
}
} // namespace

void ClickEngine::nativeMouseHover(Location loc) const {
  xcb_test_fake_input(x11inst->connection, XCB_MOTION_NOTIFY, 0,
                      XCB_CURRENT_TIME, x11inst->rootWindow, loc.x, loc.y, 0);
  xcb_flush(x11inst->connection);
}

void ClickEngine::nativeMousePress(MouseButton mouseButton, Location loc) const {
  int msbtnId = to_xcb(mouseButton);
  xcb_test_fake_input(x11inst->connection, XCB_BUTTON_PRESS, msbtnId,
                      XCB_CURRENT_TIME, x11inst->rootWindow, loc.x, loc.y, 0);
  xcb_flush(x11inst->connection);
}

void ClickEngine::nativeMouseRelease(MouseButton mouseButton, Location loc) const {
  int msbtnId = to_xcb(mouseButton);
  xcb_test_fake_input(x11inst->connection, XCB_BUTTON_RELEASE, msbtnId,
                      XCB_CURRENT_TIME, x11inst->rootWindow, loc.x, loc.y, 0);
  xcb_flush(x11inst->connection);
}

#endif
