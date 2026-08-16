#include "presets.hpp"
#include "common.hpp"
#include <QIODeviceBase>
#include <QJsonParseError>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>

#include "logger.hpp"
extern Logger *lg;

PresetManager::PresetManager(const QString& file_path) : m_file(file_path)
{
  if (!m_file.open(QIODeviceBase::ReadWrite)) {
    lg->fatal("Failed to open {}", file_path);
    return;
  }
}

bool PresetManager::load() {
  QByteArray data = m_file.readAll();
  QJsonParseError err;
  auto doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::ParseError::NoError) {
    lg->error("JSON Parsing failed: '{}', fallback to empty presets", err.errorString());
  }

  if (!doc.isObject()) {
    lg->error("JSON is invalid: must be object, fallback to empty presets");
    return true;
  }
  QJsonObject root_obj = doc.object();

  for (auto it = root_obj.constBegin(); it != root_obj.constEnd(); ++it) {
    QString name = it.key();
    QJsonValue jvConfig = it.value();
    struct PresetConfig stConfig{};

    QJsonObject joConfig;
    if (!jvConfig.isObject()) {
      lg->error("Value of {} is invalid, falling back to defaults", name);
      joConfig = QJsonObject{
        {"interval", default_interval}, {"location", QJsonArray{default_loc.x, default_loc.y}},
        {"repeat", default_repeat}, {"mouseButton", to_cstr(default_mouse_button)},
        {"currentLocation", default_current_loc}, {"repeatUntilStopped", default_repeat_forever},
      };
    } else joConfig = jvConfig.toObject();

    stConfig.interval = static_cast<size_t>(joConfig.value("interval").toInteger(default_interval));

    QJsonArray locArray = joConfig.value("location").toArray(QJsonArray{default_loc.x, default_loc.y});
    stConfig.loc.x = locArray.at(0).toInteger(default_loc.x);
    stConfig.loc.y = locArray.at(1).toInteger(default_loc.y);

    stConfig.repeat = joConfig.value("repeat").toInteger(default_repeat);
    stConfig.mouse = to_mouse_button(joConfig.value("mouseButton").toString(to_cstr(default_mouse_button)));
    stConfig.current_loc = joConfig.value("currentLocation").toBool(default_current_loc);
    stConfig.repeat_forever = joConfig.value("repeatUntilStopped").toBool(default_repeat_forever);
    m_presets.insert(name, stConfig);
  }

  return true;
}

void PresetManager::save() {
  QJsonObject obj{};
  for (const auto& [name, config] : m_presets.asKeyValueRange()) {
    QJsonObject cfg_obj{
      {"interval", static_cast<qint64>(config.interval)},
      {"location", QJsonArray{config.loc.x, config.loc.y}},
      {"repeat", config.repeat},
      {"mouseButton", to_cstr(config.mouse)},
      {"currentLocation", config.current_loc},
      {"repeatUntilStopped", config.repeat_forever},
    };
    obj[name] = cfg_obj;
  }
  m_file.resize(0);
  m_file.seek(0);
  m_file.write(QJsonDocument(obj).toJson());
}

struct PresetConfig PresetManager::get(const QString& name) const {
  return m_presets.value(name);
}

void PresetManager::set(const QString& name, const PresetConfig& cfg) {
  m_presets[name] = cfg;
}

void PresetManager::insert(const QString& name, const PresetConfig& cfg) {
  m_presets.insert(name, cfg);
}

struct PresetConfig PresetManager::take(const QString& name) {
  return m_presets.take(name);
}
