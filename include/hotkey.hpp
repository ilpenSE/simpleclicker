#pragma once
#include <QObject>
#include "common.hpp"
#include <QAbstractNativeEventFilter>

#ifdef __linux__
#include <xcb/xcb.h>
#endif

class HotkeyManager : public QObject, public QAbstractNativeEventFilter {
  Q_OBJECT

public:
  static HotkeyManager& instance(const Hotkey& init_hotkey, QObject *parent = nullptr) {
    static HotkeyManager inst(init_hotkey, parent);
    return inst;
  }
  static bool isSupportedKey(Qt::Key key);

  Hotkey hotkey() { return m_hotkey; }
  bool set(const Hotkey& hotkey);
  bool unset();

  HotkeyManager(const HotkeyManager&) = delete;
  HotkeyManager operator=(const HotkeyManager&) = delete;

signals:
  void hotkeyPressed();
  void hotkeyRegistrationFailed(const QString &reason);

protected:
  bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

private:
  Hotkey m_hotkey{};

  bool registerHotkey(const Hotkey& hotkey);
  bool unregisterHotkey();

#ifdef _WIN32
  static constexpr int HOTKEY_ID = 1;
#else // Linux X11
  xcb_keycode_t m_keycode = 0;
  quint16 m_modifiers = 0;
  xcb_connection_t *m_connection = nullptr;
  xcb_window_t m_rootWindow = 0;

#endif
  bool m_registered = false;

  explicit HotkeyManager(const Hotkey &init_hotkey, QObject *parent);
  ~HotkeyManager() override;
};
