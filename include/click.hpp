#pragma once
#include <QObject>
#include "presets.hpp"

class ClickEngine : public QObject {
  Q_OBJECT
public:
  static ClickEngine &instance(PresetConfig initPreset, QObject *parent = nullptr) {
    static ClickEngine inst(initPreset, parent);
    return inst;
  }

  PresetConfig preset() const { return m_preset; }
  void setPreset(const PresetConfig& config) { m_preset = config; }

  void start();
  void stop();

  ClickEngine(const ClickEngine&) = delete;
  ClickEngine operator=(const ClickEngine&) = delete;

  bool running = false;
private:
  PresetConfig m_preset{};

  ClickEngine(PresetConfig config, QObject *parent);
  ~ClickEngine() override;
};
