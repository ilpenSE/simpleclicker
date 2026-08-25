#pragma once

#include "common.hpp"
#include <QMap>
#include <QFile>
#include <QDir>
#include <QJsonDocument>

#define PRESET_FIELDS \
  X(location, Location, (Location{0, 0})) \
  X(interval, int, 100) \
  X(repeat, int, 1) \
  X(mouseButton, MouseButton, MouseButton::Left) \
  X(repeatUntilStopped, bool, true) \
  X(currentLocation, bool, true)

struct PresetConfig {
  #define X(Name, T, DefaultValue) T Name = DefaultValue;
  PRESET_FIELDS
  #undef X
};

class PresetManager {
public:
  static PresetManager& instance(const QString& file_path) {
    static PresetManager inst(file_path);
    return inst;
  }
  QMap<QString, PresetConfig> presets{};

  bool load();
  void save();

  PresetManager(const PresetManager&) = delete;
  PresetManager& operator =(const PresetManager&) = delete;
  PresetManager(PresetManager&&) = delete;
  PresetManager& operator =(PresetManager&&) = delete;
private:
  PresetManager(const QString& file_path);
  ~PresetManager() {
    m_file.close();
  }

  QFile m_file{};
};
