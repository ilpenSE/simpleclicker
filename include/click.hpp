#pragma once
#include <QObject>
#include "presets.hpp"
#include <QTimer>

class ClickEngine : public QObject {
  Q_OBJECT
public:
  static ClickEngine &instance(PresetConfig initPreset, QObject *parent = nullptr) {
    static ClickEngine inst(initPreset, parent);
    return inst;
  }

  PresetConfig preset() const { return m_preset; }
  void setPreset(const PresetConfig &config) {
    m_preset = config;
    m_reps = m_preset.repeat;
  }

  void start();
  void stop();

  ClickEngine(const ClickEngine&) = delete;
  ClickEngine operator=(const ClickEngine&) = delete;
  bool running = false;

signals:
  void clickFinished();

private:
  PresetConfig m_preset{};
  QTimer m_timer{};
  QTimer m_holdTimer{};
  int m_reps = 0;
  bool m_pressed = false;
  Location m_loc{};

  void click();
  void release();

  void nativeMousePress(MouseButton mouseButton, Location loc) const;
  void nativeMouseRelease(MouseButton mouseButton, Location loc) const;
  void nativeMouseHover(Location location) const;

  Location getEffectiveLocation() const {
    Location loc;
    if (m_preset.currentLocation) {
      QPoint p = QCursor::pos();
      return {.x = p.x(), .y = p.y()};
    }
    return m_preset.location;
  }

  ClickEngine(PresetConfig config, QObject *parent);
  ~ClickEngine() override;
};
