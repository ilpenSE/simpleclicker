#pragma once
#include <QFile>
#include <QJsonObject>

class SettingsManager {
public:
  static SettingsManager& instance(const QString& file_path) {
    static SettingsManager inst(file_path);
    return inst;
  }

  bool load();
  void save();

  template <typename T>
  T get(const QString& name) const {
    if constexpr (std::is_same_v<T, QString>) {
      return m_settings.value(name).toString();
    } else {
      return T{};
    }
  }

  template <typename T>
  void set(const QString& name, const T& val) {
    m_settings[name] = val;
  }

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
  QJsonObject m_settings = {
    {"currentPreset", ""},
    {"version", "1.0.0"},
    {"keybind", "F6"},
  };
};
