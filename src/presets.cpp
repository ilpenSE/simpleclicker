#include "presets.hpp"
#include "common.hpp"
#include <QIODeviceBase>
#include <QJsonParseError>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>

#include "logger.hpp"
extern Logger *lg;

PresetManager::PresetManager(const QString& file_path) : m_file(file_path), m_doc(QJsonObject{})
{
  if (!m_file.open(QIODeviceBase::ReadWrite)) {
    lg->fatal("Failed to open {}", file_path);
    return;
  }
  QByteArray data = m_file.readAll();

  QJsonParseError err;
  m_doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::ParseError::NoError) {
    lg->error("JSON Parsing failed: '{}', fallback to empty presets", err.errorString());
  }
}

bool PresetManager::deserialize() {
  if (!m_doc.isObject()) {
    lg->error("JSON is invalid: must be object, fallback to empty presets");
    m_doc = QJsonDocument(QJsonObject{});
    return true;
  }
  QJsonObject obj = m_doc.object();

  for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
    QString key = it.key();
    QJsonValue value = it.value();
    struct PresetConfig cfg{};

    QJsonObject config_obj;
    if (!value.isObject()) {
      lg->error("Value of {} is invalid, falling back to defaults", key);
      config_obj = QJsonObject{
        {"interval", default_interval}, {"location", QJsonArray{default_loc.x, default_loc.y}},
        {"repeat", default_repeat}, {"mouseButton", to_cstr(default_mouse_button)},
        {"currentLocation", default_current_loc},
      };
    } else config_obj = value.toObject();

    cfg.interval = static_cast<size_t>(config_obj.value("interval").toInteger(default_interval));

    QJsonArray locArray = config_obj.value("location").toArray(QJsonArray{default_loc.x, default_loc.y});
    cfg.loc.x = locArray.at(0).toInteger(default_loc.x);
    cfg.loc.y = locArray.at(1).toInteger(default_loc.y);

    cfg.repeat = config_obj.value("repeat").toInteger(default_repeat);
    cfg.current_loc = config_obj.value("currentLocation").toInteger(default_repeat);
    cfg.mouse = to_mouse_button(config_obj.value("mouseButton").toString(to_cstr(default_mouse_button)));
    map.insert(key, cfg);
  }

  return true;
}

bool PresetManager::serialize() {
  return true;
}
