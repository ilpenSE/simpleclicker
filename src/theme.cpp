#include "theme.hpp"
#include <QSvgRenderer>
#include <QPixmap>
#include <QPalette>
#include <QPainter>
#include <QIcon>
#include <QApplication>
#include <QFile>
#include <QDir>

QMap<QString, QColor> ThemeManager::darkColors = {
    {"icon-tint", "#e0e0e0"},
    {"window", "#1e1e1e"},
    {"window-text", "#ffffff"},
    {"base", "#181818"},
    {"alternate-base", "#282828"},
    {"text", "#ffffff"},
    {"placeholder-text", "#888888"},
    {"button", "#2d2d30"},
    {"button-text", "#ffffff"},
    {"bright-text", "#ff5555"},
    {"link", "#5a96e6"},
    {"highlight", "#264f78"},
    {"highlighted-text", "#ffffff"},
    {"tooltip-base", "#404040"},
    {"tooltip-text", "#ffffff"},
    {"disabled-text", "#7f7f7f"},
    {"disabled-button", "#1a1a1a"},

    // breeze-style shape borders
    {"border", "#4d4d4d"},
    {"border-hover", "#4287f5"},

    // preset button
    {"preset-button", QColor(255, 255, 255, 15)},
    {"preset-button-hover", QColor(255, 255, 255, 35)},
    {"preset-button-active", QColor(66, 135, 245, 15)},
    {"preset-button-active-hover", QColor(66, 135, 255, 35)},
    {"preset-button-accent", "#4287f5"},

    // notification base styles
    {"notification-info-base", "#2b6cb0"},
    {"notification-success-base", "#13700f"},
    {"notification-error-base", "#b03030"},
    {"notification-warning-base", "#b7791f"},
    {"notification-default-base", Qt::gray},

    // notification highlight
    {"notification-info-highlight", "#4299e1"},
    {"notification-success-highlight", "#22c21b"},
    {"notification-warning-highlight", "#ecc94b"},
    {"notification-error-highlight", "#f56565"},
};

QMap<QString, QColor> ThemeManager::lightColors = {
    {"window", "#f3f3f3"},
    {"window-text", "#1e1e1e"},
    {"base", "#ffffff"},
    {"alternate-base", "#ececec"},
    {"text", "#1e1e1e"},
    {"placeholder-text", "#8a8a8a"},
    {"button", "#e8e8e8"},
    {"button-text", "#1e1e1e"},
    {"bright-text", "#c0392b"},
    {"link", "#0078d4"},
    {"highlight", "#0078d4"},
    {"highlighted-text", "#ffffff"},
    {"tooltip-base", "#ffffe1"},
    {"tooltip-text", "#1e1e1e"},
    {"disabled-text", "#a0a0a0"},
    {"disabled-button", "#d8d8d8"},
    {"icon-tint", "#202020"},

    // breeze-style shape borders
    {"border", "#bcbcbc"},
    {"border-hover", "#4287f5"},

    // preset button
    {"preset-button", QColor(255, 255, 255, 90)},
    {"preset-button-hover", QColor(0, 0, 0, 15)},
    {"preset-button-active", QColor(66, 135, 245, 70)},
    {"preset-button-active-hover", QColor(66, 135, 245, 100)},
    {"preset-button-accent", "#4287f5"},

    // notification base styles
    {"notification-info-base", "#4a90d9"},
    {"notification-success-base", "#2e9e3f"},
    {"notification-error-base", "#d9534f"},
    {"notification-warning-base", "#e0a030"},
    {"notification-default-base", "#9e9e9e"},

    // notification highlight
    {"notification-info-highlight", "#6fabe6"},
    {"notification-success-highlight", "#4dbb5f"},
    {"notification-warning-highlight", "#f0c060"},
    {"notification-error-highlight", "#e57a77"},
};

ThemeManager::ThemeManager(const char *ssName, Theme initTheme, QObject *parent) : m_theme(initTheme), QObject(parent) {
  QFile ss(ssName);
  if (!ss.open(QIODevice::ReadOnly)) {
    panic("Could not open resource file: {}", ssName);
  }
  m_styleSheet = ss.readAll();
  applyTheme();
}

void ThemeManager::applyTheme() const {
  QPalette pal;

  pal.setColor(QPalette::Window,          color("window"));
  pal.setColor(QPalette::WindowText,      color("window-text"));
  pal.setColor(QPalette::Base,            color("base"));
  pal.setColor(QPalette::AlternateBase,   color("alternate-base"));
  pal.setColor(QPalette::Text,            color("text"));
  pal.setColor(QPalette::PlaceholderText, color("placeholder-text"));
  pal.setColor(QPalette::Button,          color("button"));
  pal.setColor(QPalette::ButtonText,      color("button-text"));
  pal.setColor(QPalette::BrightText,      color("bright-text"));
  pal.setColor(QPalette::Link,            color("link"));
  pal.setColor(QPalette::Highlight,       color("highlight"));
  pal.setColor(QPalette::HighlightedText, color("highlighted-text"));
  pal.setColor(QPalette::ToolTipBase,     color("tooltip-base"));
  pal.setColor(QPalette::ToolTipText,     color("tooltip-text"));

  pal.setColor(QPalette::Disabled, QPalette::Text,       color("disabled-text"));
  pal.setColor(QPalette::Disabled, QPalette::WindowText, color("disabled-text"));
  pal.setColor(QPalette::Disabled, QPalette::ButtonText, color("disabled-text"));
  pal.setColor(QPalette::Disabled, QPalette::Button,     color("disabled-button"));

  qApp->setPalette(pal);
  qApp->setStyleSheet(styleSheet());
}

QString ThemeManager::styleSheet() const {
  const QString border       = color("border").name();
  const QString borderHover  = color("border-hover").name();
  const QString accent       = color("preset-button-accent").name();
  const QString buttonBg     = color("button").name();
  const QString buttonText   = color("button-text").name();
  const QString base         = color("base").name();
  const QString altBase      = color("alternate-base").name();
  const QString disabledBg   = color("disabled-button").name();
  const QString disabledText = color("disabled-text").name();
  const QString downArrow    = arrowIconPath("down");
  const QString upArrow      = arrowIconPath("up");

  // Breeze-like stylesheet
  return m_styleSheet.arg(border, borderHover, buttonBg, buttonText)
                     .arg(accent, altBase, disabledBg, disabledText)
                     .arg(base, downArrow, upArrow);
}

QString ThemeManager::arrowIconPath(const QString& direction) const {
  const QString filename = QDir::tempPath() + QString("/breeze_arrow_%1_%2.png")
      .arg(direction, m_theme == Theme::Dark ? "dark" : "light");

  const QIcon ic = icon(direction + ".svg");
  const QPixmap pm = ic.pixmap(24, 24);
  pm.save(filename);

  return filename;
}

QIcon ThemeManager::icon(const QString &path) const {
  return icon(path, color("icon-tint"));
}

QIcon ThemeManager::icon(const QString &path, QColor tint) const {
  QSvgRenderer renderer(":/icons/" + path);
  QPixmap pixmap(22, 22);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  renderer.render(&painter);
  painter.end();

  QPixmap tinted(pixmap.size());
  tinted.fill(Qt::transparent);
  QPainter p2(&tinted);
  p2.drawPixmap(0, 0, pixmap);
  p2.setCompositionMode(QPainter::CompositionMode_SourceIn);
  p2.fillRect(tinted.rect(), tint);
  p2.end();

  return QIcon(tinted);
}
