#pragma once
#include <QFile>
#include <QJsonObject>
#include "common.hpp"
#include <type_traits>

#define SETTINGS_FIELDS \
  X(currentPreset, QString, "") \
  X(language, Language, Language::English) \
  X(keybind, Hotkey, Hotkey::from("F6")) \
  X(theme, Theme, Theme::Dark) \
  X(firstRun, bool, true) \
  X(version, Version, APP_VERSION)

template <typename T>
struct is_settings_field : std::false_type {};

#define X(Name, Type, DefaultValue)                                            \
  struct Name {                                                                \
    Type value = DefaultValue;                                                 \
  };                                                                           \
  template <> struct is_settings_field<Name> : std::true_type {};
SETTINGS_FIELDS
#undef X

template <typename T>
concept SettingsField = is_settings_field<T>::value;

class SettingsManager {
public:
  static SettingsManager& instance(const QString& file_path) {
    static SettingsManager inst(file_path);
    return inst;
  }
  bool needsUpdate = false;

  bool load();
  void save();

  template <SettingsField TFieldName>
  auto get() {
#define X(Name, Type, DefaultValue) \
    if constexpr (std::same_as<TFieldName, Name>) return fromJsonValue<Type>(settings.value(#Name), DefaultValue);
SETTINGS_FIELDS
#undef X
  }

  template <SettingsField TFieldName>
  void set(const decltype(TFieldName::value)& val) {
#define X(Name, Type, DefaultValue) \
    if constexpr (std::same_as<TFieldName, Name>) settings[#Name] = toJsonValue(val);
SETTINGS_FIELDS
#undef X
  }

  QJsonObject settings = {
#define X(Name, Type, DefaultValue) {#Name, toJsonValue(DefaultValue)},
          SETTINGS_FIELDS
#undef X
  };

  SettingsManager(const SettingsManager&) = delete;
  SettingsManager& operator =(const SettingsManager&) = delete;
  SettingsManager(SettingsManager&&) = delete;
  SettingsManager& operator =(SettingsManager&&) = delete;
private:
  SettingsManager(const QString& file_path);
  ~SettingsManager() {
    m_file.close();
  }

  QFile m_file{};
};
