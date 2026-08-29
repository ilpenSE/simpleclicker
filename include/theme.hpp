#pragma once
#include <QColor>
#include <QMap>
#include <QObject>
#include <QApplication>
#include "common.hpp"

class ThemeManager : public QObject {
  Q_OBJECT

public:
  static ThemeManager& instance(const char *ssName, Theme initTheme = Theme::Dark) {
    static ThemeManager inst(ssName, initTheme);
    return inst;
  }

  static QMap<QString, QColor> darkColors;
  static QMap<QString, QColor> lightColors;

  QString styleSheet() const;

  QColor color(const QString& var_name) const {
    if (m_theme == Theme::Dark) return darkColors.value(var_name);
    else return lightColors.value(var_name);
  }

  QIcon icon(const QString& path) const;
  QIcon icon(const QString& path, QColor color) const;

  QString arrowIconPath(const QString& direction) const;
  Theme theme() { return m_theme; }
  void setTheme(Theme theme) {
    if (m_theme == theme) return;
    m_theme = theme;
    emit themeChanged(theme);
  }

  void applyTheme() const;

  ThemeManager(const ThemeManager&) = delete;
  ThemeManager &operator=(const ThemeManager &) = delete;

signals:
  void themeChanged(Theme newTheme);

private:
  ThemeManager(const char *ssName, Theme initTheme, QObject *parent = nullptr);
  ~ThemeManager() override = default;

  QString m_styleSheet{};
  Theme m_theme = Theme::Dark;
};
