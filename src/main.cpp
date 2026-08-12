#include <QApplication>
#include <QStandardPaths>
#include <QDateTime>
#include <cstdio>

#include "mainwindow.h"
#include "logger.hpp"
#include "io.hpp"
#include "os.hpp"
#include "presets.hpp"

Logger *lg;
PresetManager *presets;

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("");
  QCoreApplication::setApplicationName("SimpleClicker");
  QString appdata_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  appdata_path += PATH_SEP;

  // Create application data folder
  if (!io::mkdir_if_not_exists(appdata_path.toUtf8().constData())) {
    printf("FATAL: Couldn't make directory: %s\n", appdata_path.toUtf8().constData());
    return 1;
  }

  // Create logs folder in appdata folder
  QString logs_folder = appdata_path + "logs" PATH_SEP_DQ;
  int ret = os::mkdir(logs_folder.toUtf8().constData(), 0775);
  if (ret != 0 && errno != EEXIST) {
    fprintf(stderr, "FATAL: Couldn't make logs directory in AppData folder\n");
    fprintf(stderr, "    which is %s\n", appdata_path.toUtf8().constData());
    return 1;
  }

  // Logger initialization
  QString time_qstr = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");
  lg = &Logger::instance();
  if (!lg->init(QString(logs_folder + time_qstr + ".log").toUtf8().constData())) {
    fprintf(stderr, "FATAL: Logger initialization failed!\n");
    return 1;
  }
  lg->info("Logger initialized");

  // Preset initialization
  QString presets_file = appdata_path + "presets.json";
  presets = &PresetManager::instance(presets_file.toUtf8().constData());
  if (!presets->init()) {
    fprintf(stderr, "FATAL: Presets initialization failed!\n");
    return 1;
  }
  lg->info("Preset manager initialized");

  MainWindow w;
  w.show();
  return QApplication::exec();
}
