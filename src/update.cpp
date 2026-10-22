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
#include <QGuiApplication>
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
#define ASSET_EXTENSION ".AppImage"
constexpr auto ASSET_NAME = "SimpleClicker-x86-64" ASSET_EXTENSION;
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
// https://github.com/ilpenSE/simpleclicker/releases/download/v1.1.0-beta/SimpleClicker-x86-64.AppImage
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
  // Get some information on environment
  const auto appImagePath = std::getenv("APPIMAGE"); // AppImage file's path
  const auto homePath = std::getenv("HOME");
  if (!homePath || !(*homePath)) {
    emit downloadFailed("Failed to get 'HOME' env variable, please set it before you ran program or you may be using some weird environment");
    return;
  }

  auto backupPath = std::string(homePath) + "/" + ASSET_NAME + ".bak";
  lg->info("Detected AppImage path = {}", appImagePath);
  lg->info("Backup path = {}", backupPath);

  if (!appImagePath || !(*appImagePath)) {
    emit downloadFailed("Failed to locate .AppImage file");
    return;
  }

  // Update .AppImage files
  if (std::rename(appImagePath, backupPath.c_str()) != 0) {
    lg->error("Rename '{}' -> '{}' failed: {}", appImagePath, backupPath, strerror(errno));
    emit downloadFailed(QString("Rename '%1' -> '%2' failed: %3").arg(appImagePath).arg(backupPath).arg(strerror(errno)));
    return;
  }

  if (std::rename(m_setupFilePath.toUtf8().constData(), appImagePath) != 0) {
    lg->error("Rename '{}' -> '{}' failed: {}", m_setupFilePath, appImagePath, strerror(errno));
    emit downloadFailed(QString("Rename '%1' -> '%2' failed: %3").arg(m_setupFilePath).arg(appImagePath).arg(strerror(errno)));
    return;
  }

  // Wait until this process dies and then run the new program
  QString command = QString("while kill -0 %1 2>/dev/null; do sleep 0.1; done; "
                              "exec \"%2\"").arg(QCoreApplication::applicationPid())
    .arg(QString::fromLocal8Bit(appImagePath));

  QProcess::startDetached("/bin/sh", {"-c", command});
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

    std::optional<Version> latest{};
    for (const auto jv : json.object().value("versions").toArray()) {
      auto name = jv.toString();
      auto ver = Version::from(name);
      if (ver == Version{}) continue;
      if (ver > APP_VERSION && (!latest || ver > *latest)) latest = ver;
    }

    if (latest) emit updateAvailable(*latest);
    else lg->info("No updates available.");
  });
}
