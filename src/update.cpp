#include "update.hpp"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QProcess>
#include <QStandardPaths>
#include "logger.hpp"
extern Logger *lg;

UpdateManager::UpdateManager(QObject *parent)
    : QObject(parent), m_netman(new QNetworkAccessManager(this)) {
  m_netman->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

// https://github.com/ilpenSE/simpleclicker/releases/download/v1.1.0-beta/SimpleClicker.exe
// https://github.com/ilpenSE/simpleclicker/releases/download/v1.1.0-beta/gnu-linux-x86-64.tar.gz
void UpdateManager::downloadAndInstall(Version version) {
  QString url = QString("https://github.com/ilpenSE/simpleclicker/releases/download/v%1/SimpleClicker.exe")
                        .arg(version.toQString());
  lg->info("Downloading: {}", url);

  QNetworkRequest request{QUrl(url)};
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
  request.setMaximumRedirectsAllowed(10);
  QNetworkReply *reply = m_netman->get(request);

  QString tempPath = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
#ifdef _WIN32
                          .filePath("SimpleClicker_setup.exe");
#else
                          .filePath("SimpleClicker_setup.tar.gz");
#endif

  auto *file = new QFile(tempPath, this);
  if (!file->open(QIODevice::WriteOnly)) {
    emit downloadFailed("Could not open temporary file: " + tempPath);
    delete file;
    return;
  }

  connect(reply, &QNetworkReply::downloadProgress, this, &UpdateManager::downloadProgress);

  connect(reply, &QIODevice::readyRead, this, [reply, file]() {
    file->write(reply->readAll());
  });

  connect(reply, &QNetworkReply::finished, this, [this, reply, file, tempPath](){
    reply->deleteLater();
    file->write(reply->readAll());
    file->close();
    file->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
      emit downloadFailed(reply->errorString());
      return;
    }

    qint64 expectedSize = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    qint64 actualSize = QFileInfo(tempPath).size();

    if (expectedSize > 0 && actualSize != expectedSize) {
      emit downloadFailed("Downloaded file has been corrupted.");
      return;
    }
    lg->info("Downloaded file to {}", tempPath);

#ifdef _WIN32
    qint64 pid = 0;
    bool started = QProcess::startDetached(tempPath, {"/VERYSILENT", "/NORESTART"}, QString(), &pid);
    if (!started) {
      emit downloadFailed(QString("Could not start installer with pid: ").arg(pid));
      return;
    }
#else
    QProcess process;

    // Unpack the tarball downloaded
    process.start("tar", {"-xzf", tempPath, "-C", "/tmp/"});
    if (!process.waitForStarted()) {
      emit downloadFailed("Could not create tar process");
      return;
    }
    if (!process.waitForFinished(-1)) {
      emit downloadFailed("Tar process exited abnormally: " + process.errorString());
      return;
    }

    // Start install script
    process.start("pkexec", {"/tmp/simpleclicker/install.sh", "/tmp/simpleclicker/"});
    if (!process.waitForStarted()) {
      emit downloadFailed("Could not create install script process");
      return;
    }
    if (!process.waitForFinished(-1)) {
      emit downloadFailed("Install script exited abnormally: " + process.errorString());
      return;
    }
    lg->info("Install script says (stdout):\n{}", QString(process.readAllStandardOutput()));
    lg->info("Install script says (stderr):\n{}", QString(process.readAllStandardError()));

    // Open upgraded program
    qint64 pid;
    process.setProgram("/usr/local/bin/SimpleClicker");
    if (!process.startDetached(&pid)) {
      lg->error("Could not open new program");
      return;
    }
#endif

    qApp->quit();
  });
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
