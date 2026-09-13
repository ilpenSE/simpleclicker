#include <QApplication>
#include <QStandardPaths>
#include <QDateTime>
#include <QObject>
#include <QFontDatabase>
#include <QFont>
#include <QLockFile>

#include "mainwindow.h"
#include "logger.hpp"
#include "presets.hpp"
#include "settings.hpp"
#include "theme.hpp"
#include "language.hpp"
#include "hotkey.hpp"
#include "click.hpp"
#include "x11.hpp"
#include "update.hpp"

Logger *lg;
PresetManager *presetsman;
SettingsManager *settingsman;
ThemeManager *thememan;
LanguageManager *langman;
HotkeyManager *hotkeyman;
ClickEngine *clickengine;
UpdateManager *updateman;
X11 *x11inst;
QLockFile *g_lockFile;

// Check if any instance of program is running
bool is_instance_running() {
  QString lockPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/simpleclicker.lock";
  g_lockFile = new QLockFile(lockPath);

  if (!g_lockFile->tryLock(100)) {
#ifdef __linux__
    qint64 pid;
    QString hostname, appname;
    if (g_lockFile->getLockInfo(&pid, &hostname, &appname)) {
      if (!QFile::exists(QString("/proc/%1").arg(pid))) {
        g_lockFile->removeStaleLockFile();
        if (!g_lockFile->tryLock(100)) return true;
      } else return true;
    } else return true;
#else // Windows already locks file if process is alive
    return true;
#endif
  }
  return false;
}

int main(int argc, char *argv[]) {
#ifdef __linux__
  // Force X11 (XCB) to global hotkeys work
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
    qputenv("QT_QPA_PLATFORM", "xcb");
  } else {
    QByteArray qpaPlatform = qgetenv("QT_QPA_PLATFORM");
    if (qpaPlatform != "xcb") {
      panic("Only X11 (XCB) is supported, try setting QT_QPA_PLATFORM=xcb environment variable or don't set it");
    }
  }
#endif

  // Force SSL backend to SChannel
#ifdef _WIN32
  qputenv("QT_SSL_BACKEND", "schannel");
#endif

  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("");
  QCoreApplication::setApplicationName("SimpleClicker");

  if (is_instance_running()) {
    panic("An instance of program is already running!");
  }

#ifdef __linux__
  x11inst = &X11::instance();
#endif

  QString appdata_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir appdata_dir(appdata_path);

  // Create application data folder
  if (!appdata_dir.mkpath(".")) {
    panic("Couldn't make directory: {}", appdata_path);
  }

  // Create logs folder in appdata folder
  QDir logs_dir(appdata_dir);
  if (!logs_dir.mkpath("logs")) {
    panic("Couldn't make 'logs' directory in AppData folder\nwhich is {}", appdata_path);
  }
  logs_dir.cd("logs");

  // Logger initialization
  lg = &Logger::instance(logs_dir);
  lg->info("Logger initialized");

  // Update manager initialization
  updateman = &UpdateManager::instance();
  lg->info("Update manager initialized");

  // Settings manager initialization
  settingsman = &SettingsManager::instance(appdata_dir.filePath("settings.json"));
  if (!settingsman->load()) return 1;
  lg->info("Settings manager initialized!");

  // Preset initialization
  presetsman = &PresetManager::instance(appdata_dir.filePath("presets.json"));
  if (!presetsman->load()) return 1;
  lg->info("Preset manager initialized");

  // Theme manager initialization
  Theme initTheme = settingsman->get<setting::theme>();
  thememan = &ThemeManager::instance(":/styles/breeze.qss", initTheme);
  QObject::connect(thememan, &ThemeManager::themeChanged, &app,
                   [&app](Theme) { thememan->applyTheme(); });
  lg->info("Theme manager initialized with {} theme", initTheme);

  app.setStyle("fusion");
  int fontId = QFontDatabase::addApplicationFont(":/fonts/Ubuntu.ttf");
  if (fontId != -1) {
    QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont font(fontFamily, 10);
    app.setFont(font);
  }

  // Language manager initialization
  Language initLang;
  if (settingsman->get<setting::firstRun>()) {
    Language sysLang = to_language(QLocale::system().language());
    lg->info("Detected system language: '{}', setting it", sysLang);
    settingsman->set<setting::language>(sysLang);
    settingsman->set<setting::firstRun>(false);
    initLang = sysLang;
  } else initLang = settingsman->get<setting::language>();
  langman = &LanguageManager::instance(initLang);
  lg->info("Language manager initialized with {} language", initLang);

  // Hotkey manager initialization
  Hotkey hotkey = settingsman->get<setting::keybind>();
  hotkeyman = &HotkeyManager::instance();
  lg->info("Hotkey manager initialized");
  if (!hotkeyman->set(hotkey)) {
    auto default_kbd = settingsman->get_default<setting::keybind>();
    lg->warning("Keybind in settings invalid, fallback to default one: {}", default_kbd);
    if (!hotkeyman->set(default_kbd)) {
      panic("Cannot set keybind to default one! Which is {}", default_kbd);
    }
    settingsman->set<setting::keybind>(default_kbd);
  }

  auto current_preset = settingsman->get<setting::currentPreset>();

  // Click engine initialization
  clickengine = &ClickEngine::instance(presetsman->presets.value(current_preset));
  lg->info("Click engine initialized");

  MainWindow w(std::move(current_preset));
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
