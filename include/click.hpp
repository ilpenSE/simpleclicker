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
  int m_reps = 0;

  void click();
  void nativeMouseClick(Location location, bool current_loc, MouseButton mouseButton) const;

  ClickEngine(PresetConfig config, QObject *parent);
  ~ClickEngine() override;
};
