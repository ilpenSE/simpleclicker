#pragma once
#include <QFile>
#include <QJsonObject>

class SettingsManager {
public:
  static SettingsManager& instance(const QString& file_path) {
    static SettingsManager inst(file_path);
    return inst;
  }
  bool needsUpdate = false;

  bool load();
  void save();

  template <typename T>
  T get(const QString& fieldName) {
    return fromJsonValue<T>(settings.value(fieldName));
  }

  template <typename T>
  void set(const QString& fieldName, const T& val) {
    settings[fieldName] = toJsonValue(val);
  }

  QJsonObject settings = {
    {"currentPreset", ""},
    {"version", "1.0.0"},
    {"keybind", "F6"},
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
