#include "presets.hpp"
#include <QIODeviceBase>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QByteArray>
#include <QJsonObject>

#include "logger.hpp"
extern Logger *lg;

bool PresetManager::init() {
  if (!m_file.open(QIODeviceBase::ReadWrite)) return false;

  QByteArray data = m_file.readAll();
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);

  if (err.error != QJsonParseError::ParseError::NoError) {
    lg->error("JSON Parsing failed: {}", err.errorString().toUtf8().constData());
    // TODO: Create fresh JSON with default values and overwrite existing one
    return false;
  }

  if (!doc.isObject()) {
    lg->error("JSON is invalid: must be object");
    // TODO: Create fresh JSON with default values and overwrite existing one
    return false;
  }

  QJsonObject obj = doc.object();
  for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
    QString key = it.key();
    QJsonValue value = it.value();
    PresetConfig cfg;

    // TODO: Make this object available but load with default values
    if (!value.isObject()) break;
    // TODO: Insert parsing logic here
    m_ps.insert(key, cfg);
  }

  return true;
}

bool PresetManager::serialize() {
  return true;
}
