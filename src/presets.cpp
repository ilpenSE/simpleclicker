#include "presets.hpp"
#include "common.hpp"
#include <QIODeviceBase>
#include <QJsonParseError>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>

#include "logger.hpp"
extern Logger *lg;

PresetManager::PresetManager(const QString& file_path)
  : m_file(file_path)
{
  if (!m_file.open(QIODeviceBase::ReadWrite)) {
    panic("Failed to open {}", file_path);
  }
}

bool PresetManager::load() {
  QByteArray data = m_file.readAll();
  QJsonParseError err;
  auto doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::ParseError::NoError) {
    lg->error("Presets JSON Parsing failed: '{}', falling back to defaults", err.errorString());
  }

  if (!doc.isObject()) {
    lg->error("Presets JSON is invalid: must be object, falling back to defaults");
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
      joConfig = {
        #define X(FieldName, _, DefaultValue) {#FieldName, toJsonValue(DefaultValue)},
        PRESET_FIELDS
        #undef X
      };
    } else joConfig = jvConfig.toObject();

#define X(FieldName, T, DefaultValue) stConfig.FieldName = fromJsonValue<T>(joConfig.value(#FieldName), DefaultValue);
PRESET_FIELDS
#undef X
    presets.insert(name, stConfig);
  }

  return true;
}

void PresetManager::save() {
  QJsonObject obj{};
  for (const auto& [presetName, config] : presets.asKeyValueRange()) {
    obj[presetName] = QJsonObject{
      #define X(FieldName, _0, _1) {#FieldName, toJsonValue(config.FieldName)},
      PRESET_FIELDS
      #undef X
    };
  }
  m_file.resize(0);
  m_file.seek(0);
  m_file.write(QJsonDocument(obj).toJson());
  lg->info("Presets saved to JSON!");
}
