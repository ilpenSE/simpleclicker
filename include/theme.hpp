#pragma once
#include <QColor>
#include <QMap>
#include <QObject>
#include <QApplication>

enum class Theme : int { Dark = 0, Light };

inline const char *to_cstr(Theme theme) {
  switch (theme) {
  case Theme::Dark: return "dark";
  case Theme::Light: return "light";
  default: return nullptr;
  }
}

inline Theme to_theme(const QString &qstr) {
  if (qstr == "dark") return Theme::Dark;
  if (qstr == "light") return Theme::Light;
  return Theme::Dark;
}

class ThemeManager : public QObject {
  Q_OBJECT

public:
  static ThemeManager& instance() {
    static ThemeManager inst;
    return inst;
  }

  static QMap<QString, QColor> darkColors;
  static QMap<QString, QColor> lightColors;

  QColor color(const QString& var_name) const {
    if (m_theme == Theme::Dark) return darkColors.value(var_name);
    else return lightColors.value(var_name);
  }

  QIcon icon(const QString& path);

  Theme theme() { return m_theme; }
  void setTheme(Theme theme) {
    if (m_theme == theme) return;
    m_theme = theme;
    emit themeChanged(theme);
  }

  void applyTheme(QApplication& app) const;

  ThemeManager(const ThemeManager&) = delete;
  ThemeManager &operator=(const ThemeManager &) = delete;

signals:
  void themeChanged(Theme newTheme);

private:
  ThemeManager(QObject *parent = nullptr) : QObject(parent) {}
  ~ThemeManager() override = default;

  Theme m_theme = Theme::Dark;
};
