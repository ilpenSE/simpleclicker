#include "update.hpp"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include "logger.hpp"
extern Logger *lg;

UpdateManager::UpdateManager(QObject *parent)
    : QObject(parent), m_netman(new QNetworkAccessManager(this)) {

}

void UpdateManager::checkForUpdates() {
  QNetworkRequest request{QUrl("https://data.jsdelivr.com/v1/package/gh/ilpenSE/simpleclicker")};

  connect(m_netman, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      lg->error("Failed to check for updates: {}", reply->errorString());
      return;
    }
    QByteArray responseData = reply->readAll();
    auto json = QJsonDocument::fromJson(responseData);

    bool update = false;
    for (const auto jv : json.object().value("versions").toArray()) {
      auto name = jv.toString();
      auto ver = Version::from(name);

      if (ver.channel == APP_VERSION.channel) {
        if (ver.major > APP_VERSION.major) update = true;
        if (ver.minor > APP_VERSION.minor) update = true;
        if (ver.patch > APP_VERSION.patch) update = true;
      }

      if (update) {
        emit updateAvailable(ver);
        break;
      }
    }
  });

  m_netman->get(request);
}
