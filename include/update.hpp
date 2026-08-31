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

signals:
  void updateAvailable(Version new_version);

private:
  void fetchEverything();
  explicit UpdateManager(QObject *parent);

  Version m_newVersion{};
  QNetworkAccessManager *m_netman;
  QList<Version> m_versions;
};
