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
#include "language.hpp"
#include "hotkey.hpp"

Logger *lg;
PresetManager *presetsman;
SettingsManager *settingsman;
ThemeManager *thememan;
LanguageManager *langman;
HotkeyManager *hotkeyman;

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

  // Settings manager initialization
  settingsman = &SettingsManager::instance(appdata_dir.filePath("settings.json"));
  if (!settingsman->load()) return 1;
  lg->info("Settings manager initialized!");

  // Preset initialization
  presetsman = &PresetManager::instance(appdata_dir.filePath("presets.json"));
  if (!presetsman->load()) return 1;
  lg->info("Preset manager initialized");

  // Theme manager initialization
  Theme initTheme = settingsman->get<Theme>("theme", Theme::Dark);
  thememan = &ThemeManager::instance(initTheme);
  QObject::connect(thememan, &ThemeManager::themeChanged, &app, [&app](Theme) {
    thememan->applyTheme();
  });
  lg->info("Theme manager initialized with {} theme", initTheme);

  // Language manager initialization
  Language initLang;
  if (settingsman->get<bool>("firstRun", true)) {
    Language sysLang = to_language(QLocale::system().language());
    lg->info("Detected system language: '{}', setting it", sysLang);
    settingsman->set("language", sysLang);
    settingsman->set("firstRun", false);
    initLang = sysLang;
  } else initLang = settingsman->get("language", Language::English);
  langman = &LanguageManager::instance(initLang);
  lg->info("Language manager initialized with {} language", initLang);

  // Hotkey manager initialization
  Hotkey hotkey = settingsman->get<Hotkey>("keybind", Hotkey::from("F6"));
  hotkeyman = &HotkeyManager::instance(hotkey);
  lg->info("Hotkey manager initialized");

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
