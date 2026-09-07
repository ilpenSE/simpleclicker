#pragma once
#include <QObject>
#include <QFile>
#include <QList>
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
  bool downloadAndInstall(Version version);
  bool checkAssetsHash(Version version);
  void install();

signals:
  void updateAvailable(Version new_version);
  void downloadProgress(qint64 received, qint64 total);
  void downloadFailed(const QString &error);

private:
  QString fetchReleaseHash(Version ver);
  explicit UpdateManager(QObject *parent);
  ~UpdateManager();

  QList<Version> m_versions;
  QString m_setupFilePath{};
  QFile *m_setupFile;
  QNetworkAccessManager *m_netman;
  Version m_newVersion{};
};
