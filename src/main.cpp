#include <QApplication>
#include <QStandardPaths>
#include <QDateTime>
#include <cstdio>

#include "mainwindow.h"
#include "logger.hpp"
#include "presets.hpp"

Logger *lg;
PresetManager *presets;

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("");
  QCoreApplication::setApplicationName("SimpleClicker");
  QString appdata_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir appdata_dir(appdata_path);

  // Create application data folder
  if (!appdata_dir.mkpath(".")) {
    printf("FATAL: Couldn't make directory: %s\n", appdata_path.toUtf8().constData());
    return 1;
  }

  // Create logs folder in appdata folder
  QDir logs_dir(appdata_dir);
  if (!logs_dir.mkpath("logs")) {
    fprintf(stderr, "FATAL: Couldn't make logs directory in AppData folder\n");
    fprintf(stderr, "    which is %s\n", appdata_path.toUtf8().constData());
    return 1;
  }
  logs_dir.cd("logs");

  // Logger initialization
  lg = &Logger::instance(logs_dir);
  lg->info("Logger initialized");

  // Preset initialization
  presets = &PresetManager::instance(appdata_dir.filePath("presets.json"));
  if (!presets->deserialize()) return 1;
  lg->info("Preset manager initialized");

  MainWindow w;
  w.show();
  return QApplication::exec();
}
