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

    std::string errm;
    if (!stConfig.validate(&errm, true)) {
      lg->error("Preset config validation failed: '{}', fallback to defaults", errm);
    }

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

bool PresetConfig::validate(std::string *err, bool fix) {
  // Repeat validating
  if (repeat <= 0) {
    if (err) *err = "Repeat count must be a positive value!";
    if (fix) repeat = get_default<preset::repeat>();
    return false;
  }

  // Interval validating
  if (interval <= 0) {
    if (err) *err = "Click interval must be a positive value!";
    if (fix) interval = get_default<preset::interval>();
    return false;
  }

  // Location validating
  Location def_loc = get_default<preset::location>();
  if (location.x < 0) {
    if (err) *err = "X dimension must be a positive value!";
    if (fix) location.x = def_loc.x;
    return false;
  }
  if (location.y < 0) {
    if (err) *err = "Y dimension must be a positive value!";
    if (fix) location.y = def_loc.y;
    return false;
  }

  if (err) *err = "<No Error>";
  return true;
}
