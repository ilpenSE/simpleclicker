#include "settings.hpp"
#include <QIODeviceBase>
#include <QJsonParseError>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>
#include <QSaveFile>
#include "logger.hpp"
extern Logger *lg;

SettingsManager::SettingsManager(const QString& file_path) : m_filePath(file_path)
{}

bool SettingsManager::load()
{
  QFile file(m_filePath);
  if (!file.exists()) return true;

  if (!file.open(QIODevice::ReadOnly)) {
    panic("Failed to open settings file: {}", file.errorString());
  }
  QByteArray data = file.readAll();
  file.close();

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
    if (!settings.contains(name)) continue;

    QJsonValue defaultValue = settings.value(name);
    if (defaultValue.type() == jvalue.type()) settings[name] = jvalue;
  }
  return true;
}

void SettingsManager::save()
{
  QSaveFile file(m_filePath);
  if (!file.open(QIODevice::WriteOnly)) {
    panic("Failed to open settings file: {}", file.errorString());
  }

  const QByteArray data = QJsonDocument(settings).toJson();
  if (file.write(data) != data.size()) {
    panic("Failed to write settings to the file: {}", file.errorString());
  }

  if (!file.commit()) {
    panic("Failed to write settings to the file: {}", file.errorString());
  }

  lg->info("Settings saved to JSON!");
}
