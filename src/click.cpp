#include "click.hpp"
#include "common.hpp"
#include "logger.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern Logger *lg;
namespace {
constexpr auto CLICK_DELAY_MS = 1;
}

ClickEngine::ClickEngine(PresetConfig config, QObject *parent)
    : m_preset(config), m_reps(m_preset.repeat), QObject(parent)
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

namespace {
static DWORD to_win32(MouseButton button, bool isPress) {
  switch (button) {
  case MouseButton::Right: return isPress ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
  case MouseButton::Middle: return isPress ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
  default: return isPress ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
  }
}

} // namespace

void ClickEngine::nativeMouseHover(Location loc) const {
  SetCursorPos(loc.x, loc.y);
}

void ClickEngine::nativeMousePress(MouseButton mouseButton, Location loc) const {
  INPUT input{.type=INPUT_MOUSE, .mi={loc.x, loc.y}};
  input.mi.dwFlags = to_win32(mouseButton, true) | MOUSEEVENTF_ABSOLUTE;
  SendInput(1, &input, sizeof(input));
}

void ClickEngine::nativeMouseRelease(MouseButton mouseButton, Location loc) const {
  INPUT input{.type=INPUT_MOUSE, .mi={loc.x, loc.y}};
  input.mi.dwFlags = to_win32(mouseButton, false) | MOUSEEVENTF_ABSOLUTE;
  SendInput(1, &input, sizeof(input));
}
