#pragma once
#include <QObject>
#include "common.hpp"
#include <QAbstractNativeEventFilter>

class HotkeyManager : public QObject, public QAbstractNativeEventFilter {
  Q_OBJECT

public:
  static HotkeyManager& instance(QObject *parent = nullptr) {
    static HotkeyManager inst(parent);
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

  static constexpr int HOTKEY_ID = 1;
  bool m_registered = false;

  explicit HotkeyManager(QObject *parent);
  ~HotkeyManager() override;
};
