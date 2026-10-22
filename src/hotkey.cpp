#include "hotkey.hpp"
#include "logger.hpp"
#include <QGuiApplication>
extern Logger *lg;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool HotkeyManager::isSupportedKey(Qt::Key key) {
  if (key >= Qt::Key_F1 && key <= Qt::Key_F35) return true;
  // Pressable key check
  return !QKeySequence(key).toString().isEmpty();
}

bool HotkeyManager::set(const Hotkey &hotkey) {
  if (!isSupportedKey(hotkey.key) || hotkey.key == 0) return false;

  if (m_registered && m_hotkey == hotkey) return true;
  if (m_registered) return false;

  if (!registerHotkey(hotkey)) return false;
  m_hotkey = hotkey;
  m_registered = true;
  lg->info("Set hotkey: {}", hotkey);
  return true;
}

bool HotkeyManager::unset() {
  if (!m_registered) {
    lg->error("Could not unset hotkey because it's not registered");
    return false;
  }
  if (!unregisterHotkey()) return false;
  m_registered = false;
  lg->info("Unset hotkey: {}", m_hotkey);
  return true;
}

namespace {
static UINT toModifiers(const Hotkey &hotkey) {
  UINT mods = 0;
  if (hotkey.ctrl)  mods |= MOD_CONTROL;
  if (hotkey.shift) mods |= MOD_SHIFT;
  if (hotkey.alt)   mods |= MOD_ALT;
  return mods;
}

static UINT toVirtualKey(Qt::Key key) {
  if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
    return VK_F1 + (key - Qt::Key_F1);
  }
  wchar_t ch = static_cast<wchar_t>(key);
  HKL layout = GetKeyboardLayout(0);
  SHORT result = VkKeyScanExW(ch, layout);
  if (result == -1) return 0;
  return LOBYTE(result);
}
} // namespace

HotkeyManager::HotkeyManager(QObject *parent) : QObject(parent) {
  qApp->installNativeEventFilter(this);
}

HotkeyManager::~HotkeyManager() {
  unset();
  qApp->removeNativeEventFilter(this);
}

bool HotkeyManager::nativeEventFilter(const QByteArray &eventType,
                                      void *message, qintptr *result) {
  if (!m_registered) return false;
  (void)eventType; (void)result;
  MSG *msg = static_cast<MSG *>(message);
  if (msg->message == WM_HOTKEY && msg->wParam == HOTKEY_ID) {
    emit hotkeyPressed();
    return true;
  }
  return false;
}

bool HotkeyManager::registerHotkey(const Hotkey& hotkey) {
  UINT vk = toVirtualKey(hotkey.key);
  UINT mods = toModifiers(hotkey) | MOD_NOREPEAT;

  bool ok = RegisterHotKey(nullptr, HOTKEY_ID, mods | MOD_NOREPEAT, vk);
  if (!ok) {
    lg->error("Could not register hotkey: Error code {}", GetLastError());
    return false;
  }
  return true;
}

bool HotkeyManager::unregisterHotkey() {
  bool ok = UnregisterHotKey(nullptr, HOTKEY_ID);
  if (!ok) {
    lg->error("Could not unregister hotkey: Error code {}", GetLastError());
    return false;
  }
  return true;
}
