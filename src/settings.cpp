#include "settings.hpp"
#include <QIODeviceBase>
#include <QJsonParseError>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>
#include "logger.hpp"
extern Logger *lg;

SettingsManager::SettingsManager(const QString& file_path)
  : m_file(file_path)
{
  if (!m_file.open(QIODeviceBase::ReadWrite)) {
    panic("Failed to open {}", file_path);
  }
}

bool SettingsManager::load() {
  QByteArray data = m_file.readAll();
  QJsonParseError err;
  auto doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::ParseError::NoError) {
    lg->error("Settings JSON Parsing failed: '{}', falling back to defaults", err.errorString());
    return true;
  }

  if (!doc.isObject()) {
    lg->error("Settings JSON is invalid: must be object, falling back to defaults");
    return true;
  }
  QJsonObject root_obj = doc.object();

  for (auto it = root_obj.constBegin(); it != root_obj.constEnd(); ++it) {
    QString name = it.key();
    QJsonValue jvalue = it.value();
    if (!settings.contains(name))
      continue;

    QJsonValue defaultValue = settings.value(name);
    if (defaultValue.type() == jvalue.type()) settings[name] = jvalue;
  }
  return true;
}

void SettingsManager::save() {
  m_file.resize(0);
  m_file.seek(0);
  m_file.write(QJsonDocument(settings).toJson());
  lg->info("Settings saved to JSON!");
}
