#include "update.hpp"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>
#include <QCryptographicHash>
#include "logger.hpp"
extern Logger *lg;

namespace {
QString sha256file(QFile *file) {
  QCryptographicHash hash(QCryptographicHash::Sha256);
  if (!hash.addData(file)) return {};
  return QString(hash.result().toHex());
}
QString sha256file(QFile &file) {
  return sha256file(&file);
}

#ifdef _WIN32
#define ASSET_EXTENSION ".exe"
constexpr auto ASSET_NAME = "SimpleClicker" ASSET_EXTENSION;
#else
#define ASSET_EXTENSION ".tar.gz"
constexpr auto ASSET_NAME = "gnu-linux-x86-64" ASSET_EXTENSION;
#endif

} // namespace

UpdateManager::UpdateManager(QObject *parent)
    : QObject(parent), m_netman(new QNetworkAccessManager(this)) {
  m_setupFilePath = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                         .filePath("SimpleClicker_setup" ASSET_EXTENSION);
  m_setupFile = new QFile(m_setupFilePath, this);
  m_netman->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

UpdateManager::~UpdateManager() {}

// https://api.github.com/repos/ilpenSE/simpleclicker/releases/tags/v$TAG
QString UpdateManager::fetchReleaseHash(Version ver) {
  QString url = QString("https://api.github.com/repos/ilpenSE/simpleclicker/releases/tags/v%1")
                       .arg(ver.toQString());
  QNetworkRequest request{QUrl(url)};
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
  request.setMaximumRedirectsAllowed(10);
  auto reply = m_netman->get(request);

  // Block until network reply is finished because we need the reply for return
  QEventLoop loop;
  connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();
  reply->deleteLater();
  if (reply->error() != QNetworkReply::NoError) {
    lg->error("Could not fetch hash of file: {}", reply->errorString());
    return "";
  }
  auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

  // Parse response JSON and get hash
  auto responseData = reply->readAll();
  if (responseData.size() <= 0) {
    lg->error("Could not fetch hash of file because GitHub API responded with nothing.");
    return "";
  }

  auto json_doc = QJsonDocument::fromJson(responseData);
  if (!json_doc.isObject()) {
    lg->error("Could not fetch hash of file because GitHub API was changed.");
    return "";
  }
  auto assets_arr = json_doc.object().value("assets").toArray();

  for (const auto& asset : assets_arr) {
    if (!asset.isObject()) continue;
    auto obj = asset.toObject();
    if (obj.value("name").toString("") != ASSET_NAME) continue;
    auto hash_str = obj.value("digest").toString("");

    QStringView hash_sv = hash_str;
    auto colon_idx = hash_sv.indexOf(':');
    if (colon_idx == -1) {
      lg->error("Could not fetch hash of file because GitHub API was changed.");
      return "";
    }
    return QString(hash_sv.slice(colon_idx + 1));
  }
  return "";
}

bool UpdateManager::checkAssetsHash(Version version) {
  // Fetch latest release binary's checksum first and compare existing file (if exists)
  // if they're not equal only then install it
  if (m_setupFile->exists()) {
    if (!m_setupFile->open(QIODevice::ReadOnly)) {
      emit downloadFailed("Could not open temporary file: " + m_setupFilePath);
      return false;
    }

    // Get hashes of files
    QString hash_remote = fetchReleaseHash(version);
    QString hash_local = sha256file(m_setupFile);

    // If they're equal, no need to install literally same asset, just install it
    if (hash_remote == hash_local) {
      return true;
    }
    m_setupFile->close();
  }
  return false;
}

// https://github.com/ilpenSE/simpleclicker/releases/download/v1.1.0-beta/SimpleClicker.exe
// https://github.com/ilpenSE/simpleclicker/releases/download/v1.1.0-beta/gnu-linux-x86-64.tar.gz
bool UpdateManager::downloadAndInstall(Version version) {
  // Open file again in write only mode for remote asset
  if (!m_setupFile->open(QIODevice::WriteOnly)) {
    emit downloadFailed("Could not open temporary file: " + m_setupFilePath);
    return false;
  }

  QString url = QString("https://github.com/ilpenSE/simpleclicker/releases/download/v%1/%2")
                        .arg(version.toQString()).arg(ASSET_NAME);

  lg->info("Downloading: {}", url);

  // Prepare request
  QNetworkRequest request{QUrl(url)};
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
  request.setMaximumRedirectsAllowed(10);
  QNetworkReply *reply = m_netman->get(request);

  connect(reply, &QNetworkReply::downloadProgress, this, &UpdateManager::downloadProgress);

  // Instantly write if a piece is ready to read
  connect(reply, &QIODevice::readyRead, this, [this, reply]() {
    m_setupFile->write(reply->readAll());
  });

  connect(reply, &QNetworkReply::finished, this, [this, reply](){
    // Consume one last piece that's ready to read
    reply->deleteLater();
    m_setupFile->write(reply->readAll());
    m_setupFile->close();
    m_setupFile->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
      emit downloadFailed(reply->errorString());
      return;
    }

    // We expect that Content-Length is equal to written file's size
    qint64 expectedSize = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    qint64 actualSize = m_setupFile->size();

    if (expectedSize > 0 && actualSize != expectedSize) {
      emit downloadFailed("Downloaded file has been corrupted.");
      return;
    }
    lg->info("Downloaded file to {}", m_setupFilePath);

    // Install
    install();
  });
  return true;
}

void UpdateManager::install() {
#ifdef _WIN32
  qint64 pid = 0;
  bool started = QProcess::startDetached(m_setupFilePath, {"/VERYSILENT", "/NORESTART"}, QString(), &pid);
  if (!started) {
    emit downloadFailed(QString("Could not start installer with pid: %1").arg(pid));
    return;
  }

#else // Linux
  QProcess process;

  // Unpack the tarball downloaded
  process.start("tar", {"-xzf", m_setupFilePath, "-C", "/tmp/"});
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
  lg->info("Install script's stdout:\n{}", QString(process.readAllStandardOutput()));
  lg->info("Install script's stderr:\n{}", QString(process.readAllStandardError()));

  const int exitCode = process.exitCode();
  // User doesn't authorized, graceful fail
  if (exitCode == 127 || exitCode == 126) return;

  // Real error
  if (exitCode != 0) {
    emit downloadFailed(QString("Install script failed with exit code %1").arg(exitCode));
    return;
  }

  // Open upgraded program
  qint64 pid;
  process.setProgram("/usr/local/bin/SimpleClicker");
  if (!process.startDetached(&pid)) {
    lg->error("Could not open new program");
    return;
  }
#endif

  qApp->quit();
}

void UpdateManager::checkForUpdates() {
  QNetworkRequest request{QUrl("https://data.jsdelivr.com/v1/package/gh/ilpenSE/simpleclicker")};
  auto reply = m_netman->get(request);

  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
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

      if (ver.channel >= APP_VERSION.channel) {
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
}
