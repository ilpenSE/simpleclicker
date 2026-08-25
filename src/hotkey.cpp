#include "hotkey.hpp"
#include "logger.hpp"
extern Logger *lg;
extern X11 *x11inst;

// TODO: Add support for Turkish keys
bool HotkeyManager::isSupportedKey(Qt::Key key) {
  if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
    return true;
  } else if (key >= Qt::Key_A && key <= Qt::Key_Z) {
    return true;
  } else if (key >= Qt::Key_0 && key <= Qt::Key_9) {
    return true;
  }
  return false;
}

bool HotkeyManager::set(const Hotkey &hotkey) {
  if (m_hotkey == hotkey) return true;
  if (m_registered) return false;
  if (!registerHotkey(hotkey)) return false;
  m_hotkey = hotkey;
  lg->info("Set hotkey: {}", hotkey);
  return true;
}

bool HotkeyManager::unset() {
  if (!m_registered) return false;
  if (!unregisterHotkey()) return false;
  m_registered = false;
  lg->info("Unset hotkey: {}", m_hotkey);
  return true;
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
namespace {
static UINT toModifiers(const Hotkey &hotkey) {
  UINT mods = 0;
  if (hotkey.ctrl)  mods |= MOD_CONTROL;
  if (hotkey.shift) mods |= MOD_SHIFT;
  if (hotkey.alt)   mods |= MOD_ALT;
  return mods;
}

static UINT toVirtualKey(Qt::Key key) {
  // ASCII and 0-9 characters
  if ((key >= Qt::Key_A && key <= Qt::Key_Z) ||
      (key >= Qt::Key_0 && key <= Qt::Key_9)) {
    return static_cast<DWORD>(key);
  }

  // Function keys
  if (key >= Qt::Key_F1 && key <= Qt::Key_F12) {
    return VK_F1 + (key - Qt::Key_F1);
  }

  switch (key) {
  case Qt::Key_Space:     return VK_SPACE;
  case Qt::Key_Tab:       return VK_TAB;
  case Qt::Key_Escape:    return VK_ESCAPE;
  case Qt::Key_Backspace: return VK_BACK;
  case Qt::Key_Delete:    return VK_DELETE;
  case Qt::Key_Insert:    return VK_INSERT;
  case Qt::Key_Home:      return VK_HOME;
  case Qt::Key_End:       return VK_END;
  case Qt::Key_PageUp:    return VK_PRIOR;
  case Qt::Key_PageDown:  return VK_NEXT;
  case Qt::Key_Up:        return VK_UP;
  case Qt::Key_Down:      return VK_DOWN;
  case Qt::Key_Left:      return VK_LEFT;
  case Qt::Key_Right:     return VK_RIGHT;
  default:                return 0;
  }
}
} // namespace

HotkeyManager::HotkeyManager(const Hotkey &init_hotkey, QObject *parent)
    : QObject(parent) {
  set(init_hotkey);
  qApp->installNativeEventFilter(this);
}

HotkeyManager::~HotkeyManager() {
  unset();
  qApp->removeNativeEventFilter(this);
}

bool HotkeyManager::nativeEventFilter(const QByteArray &eventType,
                                      void *message, qintptr *result) {
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
  m_registered = true;
  return true;
}

bool HotkeyManager::unregisterHotkey() {
  bool ok = UnregisterHotKey(nullptr, HOTKEY_ID);
  if (!ok) {
    lg->error("Could not unregister hotkey: Error code {}", GetLastError());
    return false;
  }
  m_registered = false;
  return true;
}

#else // X11

#include <xcb/xcb_keysyms.h>

namespace {
constexpr quint16 ignoredMasks[] = { 0, XCB_MOD_MASK_LOCK, XCB_MOD_MASK_2,
                                     XCB_MOD_MASK_LOCK | XCB_MOD_MASK_2 };

xcb_keycode_t toKeycode(xcb_connection_t *connection, Qt::Key key) {
  quint32 keysym;
  if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
    keysym = 0xffbe + (key - Qt::Key_F1);
  } else if (key >= Qt::Key_A && key <= Qt::Key_Z) {
    keysym = 'a' + (key - Qt::Key_A);
  } else if (key >= Qt::Key_0 && key <= Qt::Key_9) {
    keysym = '0' + (key - Qt::Key_0);
  } else {
    keysym = key;
  }

  xcb_key_symbols_t *symbols = xcb_key_symbols_alloc(connection);
  xcb_keycode_t *keycodes = xcb_key_symbols_get_keycode(symbols, keysym);
  xcb_keycode_t kc = keycodes ? keycodes[0] : 0;
  free(keycodes);
  xcb_key_symbols_free(symbols);
  return kc;
}

} // namespace

HotkeyManager::HotkeyManager(const Hotkey& init_hotkey, QObject *parent) : QObject(parent)
{
  qApp->installNativeEventFilter(this);
  set(init_hotkey);
}

HotkeyManager::~HotkeyManager() {
  unset();
  qApp->removeNativeEventFilter(this);
}

bool HotkeyManager::registerHotkey(const Hotkey& hotkey) {
  if (!x11inst->connection) return false;
  m_keycode = toKeycode(x11inst->connection, hotkey.key);
  if (m_keycode == 0) return false;

  quint16 mods = 0;
  if (hotkey.ctrl)  mods |= XCB_MOD_MASK_CONTROL;
  if (hotkey.alt)   mods |= XCB_MOD_MASK_1;
  if (hotkey.shift) mods |= XCB_MOD_MASK_SHIFT;
  m_modifiers = mods;

  for (quint16 ignored : ignoredMasks) {
    auto cookie = xcb_grab_key_checked(x11inst->connection, 0, x11inst->rootWindow,
                                       mods | ignored, m_keycode,
                                       XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    xcb_generic_error_t *err = xcb_request_check(x11inst->connection, cookie);
    if (err) {
      free(err);
      unregisterHotkey();
      emit hotkeyRegistrationFailed("This keybind probably in already use by another program");
      return false;
    }
  }
  xcb_flush(x11inst->connection);

  return true;
}

bool HotkeyManager::unregisterHotkey() {
  if (!x11inst->connection || m_keycode == 0) return false;

  for (unsigned int ignored : ignoredMasks) {
    xcb_ungrab_key(x11inst->connection, m_keycode, x11inst->rootWindow, m_modifiers | ignored);
  }
  xcb_flush(x11inst->connection);

  m_registered = false;
  return true;
}

bool HotkeyManager::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) {
  if (eventType != "xcb_generic_event_t") return false;

  xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);

  if ((event->response_type & ~0x80) == XCB_KEY_PRESS) {
    auto *keyEvent = reinterpret_cast<xcb_key_press_event_t *>(event);
    quint16 mods = keyEvent->state & (XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_CONTROL | XCB_MOD_MASK_1);

    if (keyEvent->detail == m_keycode && mods == m_modifiers) {
        emit hotkeyPressed();
        return true;
    }

  }
  return false;
}

#endif // _WIN32
