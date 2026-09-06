#pragma once
#include <QObject>
#include "common.hpp"
#include <QNetworkAccessManager>

class UpdateManager : public QObject {
  Q_OBJECT
public:
  static UpdateManager& instance(QObject *parent = nullptr) {
    static UpdateManager inst(parent);
    return inst;
  }

  void checkForUpdates();
  Version newVersion() const { return m_newVersion; }
  void downloadAndInstall(Version version);

signals:
  void updateAvailable(Version new_version);
  void downloadProgress(qint64 received, qint64 total);
  void downloadFailed(const QString &error);

private:
  explicit UpdateManager(QObject *parent);

  Version m_newVersion{};
  QNetworkAccessManager *m_netman;
  QList<Version> m_versions;
};
