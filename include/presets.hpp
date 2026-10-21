#pragma once

#include "common.hpp"
#include <QMap>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <type_traits>

#define PRESET_FIELDS \
  X(location, Location, (Location{0, 0})) \
  X(interval, int, 100) \
  X(holdTime, int, 100) \
  X(repeat, int, 1) \
  X(mouseButton, MouseButton, MouseButton::Left) \
  X(repeatUntilStopped, bool, true) \
  X(clickMode, ClickMode, ClickMode::Click) \
  X(currentLocation, bool, true)

namespace preset {
#define X(Name, T, DefaultValue) \
struct Name { \
  T value = DefaultValue; \
};
PRESET_FIELDS
#undef X
} // namespace preset

template <typename T>
struct is_preset_field : std::false_type {};

#define X(Name, Type, DefaultValue) template <> struct is_preset_field<preset::Name> : std::true_type {};
PRESET_FIELDS
#undef X

template <typename T>
concept PresetField = is_preset_field<T>::value;

struct PresetConfig {
  #define X(Name, T, DefaultValue) T Name = DefaultValue;
  PRESET_FIELDS
  #undef X

  bool validate(std::string *err = nullptr, bool fix = false);
};

template <PresetField FieldName>
auto get_default() {
  #define X(Name, T, DefaultValue) \
  if constexpr (std::same_as<FieldName, preset::Name>) return DefaultValue;
  PRESET_FIELDS
  #undef X
}

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
  ~PresetManager() {}

  QString m_filePath{};
};
