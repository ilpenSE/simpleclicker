#include <QApplication>
#include <QStandardPaths>
#include <QDateTime>
#include <cstdio>
#include <QObject>

#include "mainwindow.h"
#include "logger.hpp"
#include "presets.hpp"
#include "settings.hpp"
#include "theme.hpp"

Logger *lg;
PresetManager *presetsman;
SettingsManager *settingsman;
ThemeManager *thememan;

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

  settingsman = &SettingsManager::instance(appdata_dir.filePath("settings.json"));
  if (!settingsman->load()) return 1;
  lg->info("Settings manager initialized!");

  // Preset initialization
  presetsman = &PresetManager::instance(appdata_dir.filePath("presets.json"));
  if (!presetsman->load()) return 1;
  lg->info("Preset manager initialized");

  // Theme manager initialization
  thememan = &ThemeManager::instance();
  thememan->setTheme(to_theme(settingsman->get<QString>("theme", "dark")));
  thememan->applyTheme(app);
  QObject::connect(thememan, &ThemeManager::themeChanged, &app, [&app](Theme) {
    thememan->applyTheme(app);
  });
  lg->info("Theme manager initialized");

  MainWindow w;
  w.show();
  return QApplication::exec();
}

// On Wayland, Qt's internal allocations are considered as memory leaks by LeakSanitizer (from AddressSanitizer)
// This is a false-positive and it's considered as a bug so we suppress them in this function:
#ifdef __SANITIZE_ADDRESS__
extern "C" const char* __lsan_default_suppressions() {
  return "leak:libwayland-client.so\n" "leak:libQt6WaylandClient\n";
}
#endif
// Similar issues on GitHub:
// https://github.com/libsdl-org/SDL/issues/8056
// https://github.com/lvgl/lvgl/issues/7623
// Set QT_QPA_PLATFORM=xcb environment variable while running executable
// on Wayland to get X11 version because there's no such bug as this there.
